#include "lightning_scene.hpp"
#include "ImGuizmo.h"
#include "buffer_writer.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "material.hpp"
#include "matrix4.hpp"
#include "mesh.hpp"
#include "mesh_loader.hpp"
#include "opengl.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "shader.hpp"
#include "src/color.hpp"
#include "src/vector3.hpp"
#include "texture.hpp"
#include "window.hpp"
#include <cstring>
#include <format>
#include <gl/gl.h>
#include <memory>
#include <random>
#include <ranges>
#include <vector>


namespace
{
    struct PointLightBufer
    {
        alignas(16) game::Vector3 point;
        alignas(16) game::Color color;
        alignas(16) game::Vector3 attenuation;
    };
    struct LightBuffer
    {
        alignas(16) game::Color ambient;
        alignas(16) game::Vector3 direction;
        alignas(16) game::Color direction_color;
        int num_points;
    };

} // namespace

namespace game
{
    LightningScene::LightningScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer)
        : m_entities{}, m_ambient{0.3f, 0.3f, 0.3f}, m_directional{{0.0f, -1.0f, .0f}, {.0f, .0f, .0f}},
          m_points{{{0.0f, 5.0f, 1.0f}, {0.5f, 0.5f, 0.5f}, 1.0f, 0.07f, 0.0017f},
                   {{-5.0f, 5.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, 1.0f, 0.07f, 0.0017f}},
          m_camera{camera}, m_light_buffer{1024u}, m_renderer{renderer}


    {
        MeshLoader mesh_loader{resource_loader};
        m_texture = std::make_unique<Texture>(resource_loader.load_binary("container2.png"), 500, 500);
        m_texture_spec = std::make_unique<Texture>(resource_loader.load_binary("container2_specular.png"), 500, 500);
        m_sampler = std::make_unique<Sampler>();

        const Texture* textures[]{m_texture.get(), m_texture_spec.get()};
        const Sampler* samplers[]{m_sampler.get(), m_sampler.get()};
        const auto tex_samp = std::views::zip(textures, samplers) | std::ranges::to<std::vector>();

        const auto vertex_shader = Shader(resource_loader.load_string("shaders/lightning_vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader = Shader(resource_loader.load_string("shaders/lightning_frag.glsl"), game::ShaderType::FRAGMENT);

        m_material = std::make_unique<Material>(vertex_shader, fragment_shader);
        m_mesh = std::make_unique<Mesh>(mesh_loader.cube());

        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::uniform_real_distribution dist(-5.0f, 5.0f);

        for (auto i{-10}; i < 10; i++)
        {
            for (auto j{-10}; j < 10; j++)
            {
                m_entities.emplace_back(m_mesh.get(), m_material.get(),
                                        Vector3{static_cast<float>(i) * 2.5f, dist(gen), static_cast<float>(j) * 2.5f},
                                        Vector3{1.0f}, tex_samp);
            }
        }
    }
    void LightningScene::on_render() 
    {
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_renderer->set_camera(m_camera);

        {
            LightBuffer light_buffer{m_ambient, m_directional.direction, m_directional.color, static_cast<int>(m_points.size())};
            BufferWriter writer{m_light_buffer};
            writer.write(light_buffer);
            for (const auto& point : m_points)
            {
                auto point_light_buffer = PointLightBufer{point.position, point.color, {point.const_attenuation, point.linear_attenuation, point.quad_attenuation}};
                writer.write(point_light_buffer);
            }
        }
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_light_buffer.get_native_handle());
        for(const auto& entity : m_entities)
        {
            m_renderer->draw_mesh(entity.get_mesh(), entity.get_material(), entity.get_model_matrix(),entity.get_textures());
        }

    }
    void LightningScene::on_update(float dt) {}
    void LightningScene::on_imgui_render() 
    {
        ::ImGuiIO& io = ImGui::GetIO();
        ::ImGuizmo::SetOrthographic(false);
        ::ImGuizmo::BeginFrame();
        ::ImGuizmo::Enable(true);
        ::ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        static auto selected_point = 0;
        if (ImGui::Button("Add light"))
        {
            m_points.push_back(m_points.back());
            selected_point = m_points.size() - 1u;
        }
        if (::ImGui::CollapsingHeader("ambient"))
        {
            float colors[3]{};
            std::memcpy(colors, &m_ambient, sizeof(colors));
            if (::ImGui::ColorPicker3("ambient color", colors))
            {
                std::memcpy(&m_ambient, colors, sizeof(colors));
            }
        }
        if (::ImGui::CollapsingHeader("directional"))
        {
            float colors[3]{};
            std::memcpy(colors, &m_directional.color, sizeof(colors));
            if (::ImGui::ColorPicker3("directional color", colors))
            {
                std::memcpy(&m_directional.color, colors, sizeof(colors));
            }
        }
        for (const auto& [index, point] : m_points | std::views::enumerate)
        {
            float colors[3]{};
            std::memcpy(colors, &point.color, sizeof(colors));
            const auto header_name = std::format("color {}", index);
            const auto picker_name = std::format("color {}", index);
            const auto const_name = std::format("constant {}", index);
            const auto linear_name = std::format("linear {}", index);
            const auto quad_name = std::format("quadratic {}", index);
            if (::ImGui::CollapsingHeader(header_name.c_str()))
            {
                if (::ImGui::ColorPicker3(picker_name.c_str(), colors))
                {
                    point.color.r = colors[0];
                    point.color.g = colors[1];
                    point.color.b = colors[2];
                    selected_point = index;
                }
                ::ImGui::SliderFloat(const_name.c_str(), &point.const_attenuation, 0.0f, 1.0f);
                ::ImGui::SliderFloat(linear_name.c_str(), &point.linear_attenuation, 0.0f, 1.0f);
                ::ImGui::SliderFloat(quad_name.c_str(), &point.quad_attenuation, 0.0f, .1f);
            }
        }
        auto& point = m_points[selected_point];
        auto translate = Matrix4{point.position};
        ::ImGuizmo::Manipulate(m_camera->get_view().data(), m_camera->get_projection().data(), ::ImGuizmo::TRANSLATE,
                               ::ImGuizmo::WORLD, const_cast<float*>(translate.data().data()), nullptr, nullptr,
                               nullptr, nullptr);
        point.position.x = translate.data()[12];
        point.position.y = translate.data()[13];
        point.position.z = translate.data()[14];
    }
    void LightningScene::on_attach() 
    {
        ::glEnable(GL_DEPTH_TEST);
        ::glEnable(GL_CULL_FACE);
    }
    void LightningScene::on_detach() 
    {
    }


} // namespace game
