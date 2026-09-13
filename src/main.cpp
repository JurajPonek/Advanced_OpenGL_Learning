#include "camera.hpp"
#include <random>
#include "debug_ui.hpp"
#include "entity.hpp"
#include "error.hpp"
#include "exception.hpp"
#include "key.hpp"
#include "key_event.hpp"
#include "log.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "mesh_loader.hpp"
#include "mouse_button_evet.hpp"
#include "mouse_event.hpp"
#include "opengl.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "mesh_loader.hpp"
#include "src/vector3.hpp"
#include "stop_event.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include "vendor/opengl/glext.h"
#include "window.hpp"
#include <array>
#include <chrono>
#include <cmath>
#include <concepts>
#include <gl/gl.h>
#include <iostream>
#include <numbers>
#include <print>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

// namespace
// {


// void test()
// {
//     try
//     {

//         game::Window window{800u, 600u};
//         const auto vertex_shader = game::Shader(vertex_shader_source, game::ShaderType::VERTEX);
//         const auto fragment_shader = game::Shader(fragment_shader_source, game::ShaderType::FRAGMENT);
//         static constexpr float data[] = {
//             -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
//             0.5f, -0.5f, 0.0f, 0.0f,  1.0f,  0.0f,
//             -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  1.0f,
//             0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  1.0f,
//         };
//         static constexpr unsigned int indices[] = {3, 1, 2
//                                                     , 1, 2, 0};


//         auto material = game::Material{vertex_shader, fragment_shader};
//         ::GLuint vao{};
//         ::GLuint vbo{};
//         ::GLuint ebo{};
//         ::glGenVertexArrays(1, &vao);
//         ::glBindVertexArray(vao);
//         ::glGenBuffers(1, &vbo);
//         ::glGenBuffers(1, &ebo);
//         ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//         ::glBindBuffer(GL_ARRAY_BUFFER, vbo);
//         ::glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
//         ::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//         ::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
//         ::glEnableVertexAttribArray(0);
//         ::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
//                                 reinterpret_cast<void*>(3 * sizeof(float)));
//         ::glEnableVertexAttribArray(1);
//         auto loc = ::glGetUniformLocation(material.get_native_handle(), "c");
//         ::glUseProgram(material.get_native_handle());
//         ::glUniform4f(loc ,1.0f, 0.0f, 0.0f, 1.0f);
//         bool running = true;
//         while (running)
//         {
//             ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//             auto event = window.pump_event();
//             while (event && running)
//             {
//                 std::visit(
//                     [&](auto&& arg)
//                     {
//                         using T = std::decay_t<decltype(arg)>;
//                         if constexpr (std::same_as<T, game::StopEvent>)
//                         {
//                             running = false;
//                         }
//                     },
//                     *event);
//                 event = window.pump_event();
//             }
//             ::glUseProgram(material.get_native_handle());
//             ::glBindVertexArray(vao);
//             ::glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//             window.swap();
//         }
//     }
//     catch (game::Exception& err)
//     {
//         std::println(std::cerr, "exception {}", err);
//     }
//     catch (...)
//     {
//         std::println(std::cerr, "Unknown exception");
//     }

// }
// }


int main(int argc, char** argv)
{
    try
    {
        game::ensure(argc == 2, "./game.exe <root_path>");
        game::Window window{1920u, 1080u};
        game::ResourceLoader resource_loader{argv[1]};
        game::MeshLoader mesh_loader{resource_loader};
        game::Texture texture{resource_loader.load_binary("lambert2_BC.png"), 4096, 4096};
        //game::Texture texture_spec{resource_loader.load_binary("specular.jpg"), 4096, 4096};
        game::Sampler sampler{};
        const game::Texture* textures[]{&texture};
        const game::Sampler* samplers[]{&sampler};
        const auto tex_samp = std::views::zip(textures, samplers) | std::ranges::to<std::vector>();
        const auto vertex_shader = game::Shader(resource_loader.load_string("vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader =
            game::Shader(resource_loader.load_string("frag.glsl"), game::ShaderType::FRAGMENT);
        auto material = game::Material{vertex_shader, fragment_shader};
        auto mesh = game::Mesh{mesh_loader.load("revolver.fbx")};
        const auto renderer = game::Renderer{};
        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::uniform_real_distribution dist(-5.0f, 5.0f);
        std::vector<game::Entity> entities{};
        entities.emplace_back(&mesh, &material,
                                     game::Vector3{1.0f, 1.0f, 1.0f}, game::Vector3{1.0f},
                                       tex_samp);
        // for (auto i{-10}; i < 10; i++)
        // {
        //     for (auto j{-10}; j < 10; j++)
        //     {
        //         entities.emplace_back(&mesh, &material,
        //                               game::Vector3{static_cast<float>(i) * 2.5f, dist(gen), static_cast<float>(j) * 2.5f}, game::Vector3{1.0f},
        //                               tex_samp);
        //     }
        // }

        auto scene = game::Scene{entities | std::views::transform([](const auto& e) { return &e; }) |
                                     std::ranges::to<std::vector>(),
                                 {0.3f, 0.3f, 0.3f},
                                 {{0.0f, -1.0f, .0f}, {.0f, .0f, .0f}},
                                 {{{0.0f, 5.0f, 1.0f}, {0.5f, 0.5f, 0.5f}, 1.0f, 0.07f, 0.0017f},
                                  {{-5.0f, 5.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, 1.0f, 0.07f, 0.0017f}
                                
                                }};

        auto camera = game::Camera{{0.0f, 0.0f, 6.0f},
                                   {0.0f, 1.0f, 0.0f},
                                   {0.0f, 1.0f, 0.0f},
                                   std::numbers::pi_v<float> / 4,
                                   static_cast<float>(window.get_width()),
                                   static_cast<float>(window.get_height()),
                                   0.1,
                                   100.0f};
        auto running = true;
        auto show_debug = true;
        auto key_states = std::unordered_map<game::Key, bool>{};
        auto last_time = std::chrono::high_resolution_clock::now();
        float speed = 20.0f;
        const game::DebugUI ui{window.get_native_handle(), scene, camera};

        while (running)
        {
            auto current_time = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;
            auto event = window.pump_event();
            while (event && running)
            {
                std::visit(
                    [&](auto&& arg)
                    {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::same_as<T, game::StopEvent>)
                        {
                            running = false;
                        }
                        else if constexpr (std::same_as<T, game::KeyEvent>)
                        {
                            if (arg.get_key() == game::Key::ESC)
                            {
                                running = false;
                            }
                            key_states[arg.get_key()] = arg.get_state() == game::KeyState::DOWN ? true : false;
                            if (arg.get_key() == game::Key::TAB && arg.get_state() == game::KeyState::DOWN)
                            {
                                show_debug = !show_debug;
                            }
                        }
                        else if constexpr (std::same_as<T, game::MouseEvent>)
                        {
                            if (!show_debug)
                            {
                                static constexpr float sensitivity = 0.001f;
                                const float delta_x = arg.get_delta_x() * sensitivity;
                                const float delta_y = arg.get_delta_y() * sensitivity;
                                camera.adjust_yaw(delta_x);
                                camera.adjust_pitch(-delta_y);
                            }
                        }
                        else if constexpr (std::is_same_v<T, game::MouseButtonEvent>)
                        {
                            ui.add_mouse_event(arg);
                        }
                    }

                    ,
                    *event);
                event = window.pump_event();
            }
            auto walk_direction = game::Vector3{0.0f, 0.0f, 0.0f};
            if (key_states[game::Key::D])
            {
                walk_direction += camera.get_right();
            }
            if (key_states[game::Key::A])
            {
                walk_direction -= camera.get_right();
            }
            if (key_states[game::Key::W])
            {
                walk_direction += camera.get_direction();
            }
            if (key_states[game::Key::S])
            {
                walk_direction -= camera.get_direction();
            }
            walk_direction = game::Vector3::normalize(walk_direction);
            camera.translate(game::Vector3::normalize(walk_direction) * speed * dt);
            renderer.render(camera, scene);
            if (show_debug)
            {

                ui.render();
            }
            window.swap();
        }
    }
    catch (game::Exception& err)
    {
        std::println(std::cerr, "exception {}", err);
    }
    catch (...)
    {
        std::println(std::cerr, "Unknown exception");
    }
    // test();
    return 0;
}
