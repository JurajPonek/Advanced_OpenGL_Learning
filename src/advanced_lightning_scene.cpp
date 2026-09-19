#include "advanced_lightning_scene.hpp"
#include <ImGuizmo.h>
#include "buffer_writer.hpp"
#include "color.hpp"
#include "framebuffer.hpp"
#include "imgui.h"
#include "matrix4.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include "vendor/opengl/glext.h"
#include <cstring>
#include <format>
#include <gl/gl.h>
#include <memory>

namespace
{
    bool g_enable_msaa = true;
    struct PointLightBuffer
    {
        alignas(16) game::Vector3 position{};
        alignas(16) game::Color color{};
        alignas(16) game::Vector3 attenuation{};
    };

    struct LightBuffer
    {
        alignas(16) game::Color ambient;
        alignas(16) game::Vector3 direction{};
        alignas(16) game::Color dir_color{};
        int num_points{};
    };
} // namespace


namespace game
{
    AdvancedLightningScene::AdvancedLightningScene(ResourceLoader& resource_loader, Window* window, Camera* camera,
                                                   Renderer* renderer, MeshLoader* mesh_loader)
        : m_entities{}, m_camera{camera}, m_renderer{renderer}, m_mesh_loader{mesh_loader},
          m_points{{{0.0f, 5.0f, 1.0f}, {0.5f, 0.5f, 0.5f}, 1.0f, 0.07f, 0.0017f}}, m_light_buffer{1024u},
          m_directional{{0.0f, -1.0f, .0f}, {.0f, .0f, .0f}}, m_ambient{0.3f, 0.3f, 0.3f}
    {
        FramebufferSpecification spec{};
        spec.width = window->get_width();
        spec.height = window->get_height();
        spec.samples = 8;
        spec.attachments = {TextureUsage::COLORATTACHMENT, TextureUsage::DEPTHATTACHMENT};
        m_msaa_fbo = std::make_unique<FrameBuffer>(spec);
        spec.samples = 1;
        m_post_process_fbo = std::make_unique<FrameBuffer>(spec);

        m_default_texture = std::make_unique<Texture>(resource_loader.load_binary("container2.png"));
        m_plane_texture = std::make_unique<Texture>(resource_loader.load_binary("wooden_floor.png"));
        m_sampler = std::make_unique<Sampler>();
        const Texture* textures1[]{m_default_texture.get()};
        const Sampler* samplers1[]{m_sampler.get()};
        const Texture* textures2[]{m_plane_texture.get()};
        const Sampler* samplers2[]{m_sampler.get()};
        const auto tex_samp1 = std::views::zip(textures1, samplers1) | std::ranges::to<std::vector>();
        const auto tex_samp2 = std::views::zip(textures2, samplers2) | std::ranges::to<std::vector>();

        const auto vertex_shader =
            Shader(resource_loader.load_string("shaders/advanced_lightning_vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader =
            Shader(resource_loader.load_string("shaders/advanced_lightning_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto post_process_vert =
            Shader(resource_loader.load_string("shaders/post_process_vert.glsl"), game::ShaderType::VERTEX);
        const auto post_process_frag =
            Shader(resource_loader.load_string("shaders/post_process_frag.glsl"), game::ShaderType::FRAGMENT);
        m_material = std::make_unique<Material>(vertex_shader, fragment_shader);
        m_post_process_material = std::make_unique<Material>(post_process_vert, post_process_frag);
        m_cube = std::make_unique<Mesh>(m_mesh_loader->cube());
        m_plane = std::make_unique<Mesh>(m_mesh_loader->plane());
        m_sphere = std::make_unique<Mesh>(m_mesh_loader->sphere());
        m_entities.emplace_back(m_cube.get(), m_material.get(), Vector3{1.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp1);
        m_entities.emplace_back(m_plane.get(), m_material.get(), Vector3{0.0f, 0.0f, 0.0f}, Vector3{10.0f, 1.0f, 10.0f},
                                tex_samp2);
        m_entities.emplace_back(m_sphere.get(), m_material.get(), Vector3{5.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp1);
    }
    void AdvancedLightningScene::on_render()
    {
        FrameBuffer* render_target = g_enable_msaa ? m_msaa_fbo.get() : m_post_process_fbo.get();
        render_target->bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glEnable(GL_DEPTH_TEST);
        m_renderer->set_camera(m_camera);
        setup_lights();
        for (const auto& entity : m_entities)
        {
            m_renderer->draw_mesh(entity.get_mesh(), entity.get_material(), entity.get_model_matrix(),
                                  entity.get_textures());
        }
        render_target->unbind();
        if (g_enable_msaa)
        {
            ::glBlitNamedFramebuffer(m_msaa_fbo->get_native_handle(), m_post_process_fbo->get_native_handle(), 0, 0,
                                     m_msaa_fbo->get_width(), m_msaa_fbo->get_height(), 0, 0,
                                     m_post_process_fbo->get_width(), m_post_process_fbo->get_height(),
                                     GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glDisable(GL_DEPTH_TEST);
        m_renderer->draw_post_process_texture(m_post_process_material.get(), m_sampler.get(), m_post_process_fbo.get());
    }
    void AdvancedLightningScene::on_imgui_render() 
    { 
        ::ImGui::Checkbox("MSAA", &g_enable_msaa);
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
    void AdvancedLightningScene::on_attach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
    void AdvancedLightningScene::on_detach() { ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0); }
    void AdvancedLightningScene::setup_lights() const
    {
        LightBuffer light_buffer{m_ambient, m_directional.direction, m_directional.color, static_cast<int>(m_points.size())};
        BufferWriter writer{m_light_buffer};
        writer.write(light_buffer);
        for (const auto& point : m_points)
        {
            PointLightBuffer point_buffer = {
                point.position,
                point.color,
                {point.const_attenuation, point.linear_attenuation, point.quad_attenuation}};
            writer.write(point_buffer);
        }
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_light_buffer.get_native_handle());
    }
} // namespace game