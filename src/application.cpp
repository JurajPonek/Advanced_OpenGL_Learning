#include "application.hpp"
#include "camera.hpp"
#include "clear_color_scene.hpp"
#include "exception.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "key.hpp"
#include "key_event.hpp"
#include "lightning_scene.hpp"
#include "mesh_loader.hpp"
#include "mouse_button_evet.hpp"
#include "mouse_event.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "scene.hpp"
#include "src/frame_buffer_scene.hpp"
#include "src/instancing_scene.hpp"
#include "src/vector3.hpp"
#include "stop_event.hpp"
#include "window.hpp"
#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <numbers>
#include <print>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <iostream>

namespace game
{
    Application::Application() : m_window{1920u, 1080u}
    {
        ::IMGUI_CHECKVERSION();
        ::ImGui::CreateContext();
        ::ImGui::StyleColorsDark();
        ::ImGui_ImplWin32_InitForOpenGL(m_window.get_native_handle());
        ::ImGui_ImplOpenGL3_Init();
    }

    Application::~Application()
    {
        ::ImGui_ImplOpenGL3_Shutdown();
        ::ImGui_ImplWin32_Shutdown();
        ::ImGui::DestroyContext();
    }

    void Application::run() 
    {
        try
        {
            ResourceLoader resource_loader{"../assets/"};
            MeshLoader mesh_loader{resource_loader};
            auto camera = Camera{{0.0f, 0.0f, 6.0f},
                                       {0.0f, 1.0f, 0.0f},
                                       {0.0f, 1.0f, 0.0f},
                                       std::numbers::pi_v<float> / 4,
                                       static_cast<float>(m_window.get_width()),
                                       static_cast<float>(m_window.get_height()),
                                       0.1,
                                       1000.0f};
            SceneManager manager{};
            Renderer renderer{mesh_loader, resource_loader,};

            manager.add_scene<ClearColorScene>("ClearColorScene");
            manager.add_scene<LightningScene>("LightningScene", resource_loader, &m_window, &camera, &renderer);
            manager.add_scene<FrameBufferScene>("FrameBufferScene", resource_loader, &m_window, &camera, &renderer, &mesh_loader);
            manager.add_scene<InstancingScene>("InstancingScene", resource_loader, &m_window, &camera, &renderer, &mesh_loader);
            manager.set_startup_scene([&]()
                { return std::make_unique<InstancingScene>(resource_loader, &m_window, &camera, &renderer, &mesh_loader); });


            auto running = true;
            auto key_states = std::unordered_map<Key, bool>{};
            auto last_time = std::chrono::high_resolution_clock::now();
            float speed = 20.0f;
            bool show_debug = true;
            while (running)
            {
                auto current_time = std::chrono::high_resolution_clock::now();
                float dt = std::chrono::duration<float>(current_time - last_time).count();
                last_time = current_time;
                auto event = m_window.pump_event();
                while (event && running)
                {
                    std::visit(
                        [&](auto&& arg)
                        {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::same_as<T, StopEvent>)
                            {
                                running = false;
                            }
                            else if constexpr (std::same_as<T, KeyEvent>)
                            {
                                if (arg.get_key() == Key::ESC)
                                {
                                    running = false;
                                }
                                key_states[arg.get_key()] = arg.get_state() == KeyState::DOWN ? true : false;
                                if (arg.get_key() == Key::TAB && arg.get_state() == game::KeyState::DOWN)
                                {
                                    show_debug = !show_debug;
                                }
                            }
                            else if constexpr (std::same_as<T, MouseEvent>)
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
                            else if constexpr (std::is_same_v<T, MouseButtonEvent>)
                            {
                                ImGuiIO& io = ImGui::GetIO();
                                io.AddMouseButtonEvent(0, arg.get_state() == MouseButtonState::DOWN);
                            }
                        },
                        *event);
                    event = m_window.pump_event();
                }
                auto walk_direction = Vector3{0.0f, 0.0f, 0.0f};
                if (key_states[Key::D])
                {
                    walk_direction += camera.get_right();
                }
                if (key_states[Key::A])
                {
                    walk_direction -= camera.get_right();
                }
                if (key_states[Key::W])
                {
                    walk_direction += camera.get_direction();
                }
                if (key_states[Key::S])
                {
                    walk_direction -= camera.get_direction();
                }
                walk_direction = Vector3::normalize(walk_direction);
                camera.translate(Vector3::normalize(walk_direction) * speed * dt);

                if (!running)
                    break;
                manager.update(dt);

                manager.render();
                if (show_debug)
                {
                    manager.render_ui();
                }
                m_window.swap();
            }
        }
        catch (Exception& err)
        {
            std::println(std::cerr, "exception {}", err);
        }
        catch (...)
        {
            std::println(std::cerr, "Unknown exception");
        }
    }


} // namespace game