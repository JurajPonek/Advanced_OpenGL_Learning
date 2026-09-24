#include "advanced_lightning_scene.hpp"
#include "buffer_writer.hpp"
#include "color.hpp"
#include "framebuffer.hpp"
#include "imgui.h"
#include "matrix4.hpp"
#include "opengl.hpp"
#include "sampler.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include "vendor/opengl/glext.h"
#include <ImGuizmo.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <gl/gl.h>
#include <memory>
#include <numbers>
#include <ranges>
#include <span>
#include <vector>


namespace
{
    bool g_enable_msaa = true;
    bool g_use_normal_map = true;
    float g_gamma = 2.2f;
    struct PointLightBuffer
    {
        alignas(16) game::Vector3 position{};
        alignas(16) game::Color color{};
        int shininess{};
        float radius{};
        float intensity{};
        int shadow_map_index;
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
    static constexpr std::uint32_t SHADOW_MAP_WIDTH = 2048;
    static constexpr std::uint32_t SHADOW_MAP_HEIGHT = 2048;
    static constexpr int MAX_POINT_LIGHTS = 6;
    AdvancedLightningScene::AdvancedLightningScene(ResourceLoader& resource_loader, Window* window, Camera* camera,
                                                   Renderer* renderer, MeshLoader* mesh_loader)
        : m_entities{}, m_camera{camera}, m_renderer{renderer}, m_mesh_loader{mesh_loader}, m_points{},
          m_light_buffer{10240u}, m_directional{{0.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}, m_ambient{0.3f, 0.3f, 0.3f}
    {
        
        


        FramebufferSpecification spec{};
        spec.width = window->get_width();
        spec.height = window->get_height();
        spec.samples = 8;
        spec.type = TextureType::TEXTURE2D;
        spec.attachments = {TextureFormat::RGBA8, TextureFormat::Depth32F};
        m_msaa_fbo = std::make_unique<FrameBuffer>(spec);

        spec.samples = 1;
        m_post_process_fbo = std::make_unique<FrameBuffer>(spec);

        spec.attachments = {TextureFormat::Depth32F};
        spec.width = SHADOW_MAP_WIDTH;
        spec.height = SHADOW_MAP_HEIGHT;
        m_shadow_map = std::make_unique<FrameBuffer>(spec);

        spec.attachments = {TextureFormat::Depth32F};
        spec.max_lights = MAX_POINT_LIGHTS;
        spec.type = TextureType::DEPTHCUBEMAP;
        m_omnidirectional_shadow_map = std::make_unique<FrameBuffer>(spec);
        m_shadow_proj =
            Matrix4::perspective(std::numbers::pi_v<float> / 2.0f, m_omnidirectional_shadow_map->get_width(),
                                 m_omnidirectional_shadow_map->get_height(), 1.0f, 25.0f);

        m_points.emplace_back(PointLight{
            {0.0f, 5.0f, 1.0f}, {0.5f, 0.5f, 0.5f}, 64, 25.0f, 30.0f, static_cast<int>(m_points.size())});

        m_default_texture =
            std::make_unique<Texture>(resource_loader.load_binary("container2.png"), TextureFormat::SRGBA);
        m_plane_texture =
            std::make_unique<Texture>(resource_loader.load_binary("wooden_floor.png"), TextureFormat::SRGBA);
        m_brick_texture =
            std::make_unique<Texture>(resource_loader.load_binary("brickwall.jpg"), TextureFormat::SRGBA);
        m_brick_normal_map =
            std::make_unique<Texture>(resource_loader.load_binary("brickwall_normal.jpg"), TextureFormat::SRGBA);

        TextureSpecification default_normal_map_spec;
        default_normal_map_spec.default_normal_map_texture = true;
        default_normal_map_spec.type = TextureType::TEXTURE2D;
        m_default_normal_map_texture = std::make_unique<Texture>(default_normal_map_spec);

        m_sampler = std::make_unique<Sampler>();
        m_shadow_map_sampler = std::make_unique<Sampler>(SamplerUsage::SHADOWMAP);
        const Texture* textures1[]{m_default_texture.get(), m_default_normal_map_texture.get()}; 
        const Sampler* samplers[] = {m_sampler.get(), m_sampler.get()};
        const Texture* textures2[]{m_plane_texture.get(), m_default_normal_map_texture.get()};
        const Texture* textures3[]{m_brick_texture.get(), m_brick_normal_map.get()};
        const Texture* textures4[]{m_brick_texture.get(), m_default_normal_map_texture.get()};

        const auto tex_samp1 = std::views::zip(textures1, samplers) | std::ranges::to<std::vector>();
        const auto tex_samp2 = std::views::zip(textures2, samplers) | std::ranges::to<std::vector>();
        const auto tex_samp3 = std::views::zip(textures3, samplers) | std::ranges::to<std::vector>();
        const auto tex_samp4 = std::views::zip(textures4, samplers) | std::ranges::to<std::vector>();

        const auto vertex_shader =
            Shader(resource_loader.load_string("shaders/advanced_lightning_vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader =
            Shader(resource_loader.load_string("shaders/advanced_lightning_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto shadow_map_vert =
            Shader(resource_loader.load_string("shaders/shadow_map_vert.glsl"), game::ShaderType::VERTEX);
        const auto shadow_map_frag =
            Shader(resource_loader.load_string("shaders/shadow_map_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto post_process_vert =
            Shader(resource_loader.load_string("shaders/post_process_vert.glsl"), game::ShaderType::VERTEX);
        const auto post_process_frag =
            Shader(resource_loader.load_string("shaders/post_process_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto point_shadows_vert =
            Shader(resource_loader.load_string("shaders/point_shadows_vert.glsl"), game::ShaderType::VERTEX);
        const auto point_shadows_geo =
            Shader(resource_loader.load_string("shaders/point_shadows_geo.glsl"), game::ShaderType::GEOMETRY);
        const auto point_shadows_frag =
            Shader(resource_loader.load_string("shaders/point_shadows_frag.glsl"), game::ShaderType::FRAGMENT);

        m_material = std::make_unique<Material>(vertex_shader, fragment_shader);
        m_post_process_material = std::make_unique<Material>(post_process_vert, post_process_frag);
        m_shadow_map_material = std::make_unique<Material>(shadow_map_vert, shadow_map_frag);
        m_point_shadows_material =
            std::make_unique<Material>(point_shadows_vert, point_shadows_geo, point_shadows_frag);
        m_cube = std::make_unique<Mesh>(m_mesh_loader->cube());
        m_plane = std::make_unique<Mesh>(m_mesh_loader->plane());
        m_sphere = std::make_unique<Mesh>(m_mesh_loader->sphere());
        m_entities.emplace_back(m_cube.get(), m_material.get(), Vector3{1.0f, 4.0f, 1.0f}, Vector3{1.0f}, tex_samp1);
        m_entities.emplace_back(m_plane.get(), m_material.get(), Vector3{0.0f, 0.0f, 0.0f}, Vector3{10.0f, 1.0f, 10.0f},
                                tex_samp2);
        m_entities.emplace_back(m_sphere.get(), m_material.get(), Vector3{5.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp1);
        m_entities.emplace_back(m_plane.get(), m_material.get(), Vector3{20.0f, 0.0f, 0.0f}, Vector3{10.0f, 1.0f, 10.0f}, 
        tex_samp3);
        m_entities.emplace_back(m_plane.get(), m_material.get(), Vector3{40.0f, 0.0f, 0.0f}, Vector3{10.0f, 1.0f, 10.0f},tex_samp4);
    }
    void AdvancedLightningScene::on_render()
    {
        FrameBuffer* render_target = g_enable_msaa ? m_msaa_fbo.get() : m_post_process_fbo.get();

        ::glEnable(GL_DEPTH_TEST);
        m_shadow_map->bind();
        ::glClear(GL_DEPTH_BUFFER_BIT);
        Vector3 light_dir = Vector3::normalize(m_directional.direction);
        Vector3 scene_center = {0.0f, 0.0f, 0.0f};
        float shadow_distance = 10.0f;
        Vector3 light_pos = scene_center - light_dir * shadow_distance;
        float near_plane = 1.0f, far_plane = 50.0f;
        Matrix4 light_view = Matrix4::look_at(light_pos, scene_center, {0.0f, 1.0f, 0.0f});
        Matrix4 light_projection = Matrix4::orthographic(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);
        Matrix4 light_space_matrix = light_projection * light_view;
        for (const auto& entity : m_entities)
        {
            m_renderer->draw_to_depth_buffer(entity.get_mesh(), m_shadow_map_material.get(), entity.get_model_matrix(),
                                             light_space_matrix);
        }
        m_shadow_map->unbind();

        ::glEnable(GL_DEPTH_TEST);
        m_omnidirectional_shadow_map->bind();
        m_point_shadows_material->use();
        m_point_shadows_material->set_uniform("far_plane", 25.0f);
        ::glClear(GL_DEPTH_BUFFER_BIT);
        for (const auto& point : m_points)
        {
            const auto& transforms = calculate_shadow_transformations(point);
            m_point_shadows_material->set_uniform("light_index", point.shadow_map_index);
            m_point_shadows_material->set_uniform("light_pos", point.position);
            m_point_shadows_material->set_uniform("shadow_matrices[0]", std::span{transforms});
            for (const auto& entity : m_entities)
            {
                const auto* mesh = entity.get_mesh();
                m_point_shadows_material->set_uniform("model", entity.get_model_matrix());
                mesh->bind();
                ::glDrawElements(GL_TRIANGLES, mesh->get_index_count(), GL_UNSIGNED_INT,
                                 reinterpret_cast<void*>(mesh->get_index_offset()));
                mesh->unbind();
            }
        }
        m_omnidirectional_shadow_map->unbind();


        render_target->bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glEnable(GL_DEPTH_TEST);
        m_renderer->set_camera(m_camera);
        setup_lights();
        setup_shadows(light_space_matrix);
        setup_point_shadows();
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
        m_post_process_material->set_uniform("gamma", g_gamma);
        m_renderer->draw_post_process_texture(m_post_process_material.get(), m_sampler.get(), m_post_process_fbo.get());
    }
    void AdvancedLightningScene::on_imgui_render()
    {
        ::ImGui::Checkbox("MSAA", &g_enable_msaa);
        ::ImGui::Checkbox("NORMAL_MAPS", &g_use_normal_map);
        ::ImGui::SliderFloat("Gamma", &g_gamma, 0.0f, 5.0f);
        ::ImGuiIO& io = ImGui::GetIO();
        ::ImGuizmo::SetOrthographic(false);
        ::ImGuizmo::BeginFrame();
        ::ImGuizmo::Enable(true);
        ::ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        static auto selected_point = 0;
        if (ImGui::Button("Add light"))
        {
            const auto& last = m_points.back();
            m_points.emplace_back(PointLight{last.position,
                                             last.color,
                                             last.shininess,
                                             last.radius,
                                             last.intensity,
                                            static_cast<int>(m_points.size())});
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
            float dir[3]{};
            std::memcpy(colors, &m_directional.color, sizeof(colors));
            std::memcpy(dir, &m_directional.direction, sizeof(dir));
            if (::ImGui::SliderFloat3("Direction", dir, -10.0f, 10.0f))
            {
                std::memcpy(&m_directional.direction, dir, sizeof(dir));
            };
            if (::ImGui::ColorPicker3("directional color", colors))
            {
                std::memcpy(&m_directional.color, colors, sizeof(colors));
            }
        }
        for (const auto& [index, point] : m_points | std::views::enumerate)
        {
            float colors[3]{};
            std::memcpy(colors, &point.color, sizeof(colors));
            const auto header_name = std::format("pointlight {}", index);
            const auto picker_name = std::format("color {}", index);
            const auto shininess_name = std::format("shininess {}", index);
            const auto radius_name = std::format("radius {}", index);
            const auto intensity_name = std::format("intensity {}", index);
            if (::ImGui::CollapsingHeader(header_name.c_str()))
            {
                if (::ImGui::ColorPicker3(picker_name.c_str(), colors))
                {
                    point.color.r = colors[0];
                    point.color.g = colors[1];
                    point.color.b = colors[2];
                    selected_point = index;
                }

                ::ImGui::SliderInt(shininess_name.c_str(), &point.shininess, 0, 128);
                ::ImGui::SliderFloat(radius_name.c_str(), &point.radius, 0.0f, 100.0f);
                ::ImGui::SliderFloat(intensity_name.c_str(), &point.intensity, 0.0f, 100.0f);
            }
        }
        auto view = m_camera->get_view();
        auto projection = m_camera->get_projection();
        auto& point = m_points[selected_point];
        auto translate = Matrix4{point.position};
        ::ImGuizmo::Manipulate(view.data(), projection.data(), ::ImGuizmo::TRANSLATE, ::ImGuizmo::WORLD,
                               const_cast<float*>(translate.data().data()), nullptr, nullptr, nullptr, nullptr);
        point.position.x = translate.data()[12];
        point.position.y = translate.data()[13];
        point.position.z = translate.data()[14];
        ImGui::Begin("Framebuffer Preview");

        ::GLuint texture_id = m_shadow_map->get_depth_attachment().get_native_handle();

        ImVec2 image_size = ImVec2(320.0f, 180.0f);
        ImTextureID imgui_texture_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id));

        ImGui::Image(imgui_texture_id, image_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        // ImGui::Text("FBO size: %dx%d", m_fbo->get_width(), m_fbo->get_height());


        ImGui::End();
    }
    void AdvancedLightningScene::on_attach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
    void AdvancedLightningScene::on_detach() { ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0); }
    void AdvancedLightningScene::setup_lights() const
    {
        LightBuffer light_buffer{m_ambient, m_directional.direction, m_directional.color,
                                 static_cast<int>(m_points.size())};
        BufferWriter writer{m_light_buffer};
        writer.write(light_buffer);
        for (const auto& point : m_points)
        {
            PointLightBuffer point_buffer = {point.position, point.color, point.shininess, point.radius,
                                             point.intensity, point.shadow_map_index};
            writer.write(point_buffer);
        }
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_light_buffer.get_native_handle());
    }
    void AdvancedLightningScene::setup_shadows(const Matrix4& lightSpaceMatrix)
    {
        m_material->use();
        m_material->set_uniform("light_space_matrix", lightSpaceMatrix);
        m_material->set_uniform("use_normal_map", g_use_normal_map);
        m_material->bind_texture(2, &m_shadow_map->get_depth_attachment(), m_shadow_map_sampler.get());
    }
    std::array<Matrix4, 6> AdvancedLightningScene::calculate_shadow_transformations(const PointLight& point) const
{
    return {
        m_shadow_proj * Matrix4::look_at(point.position, point.position + Vector3( 1.0,  0.0,  0.0), Vector3(0.0, -1.0,  0.0)),
        m_shadow_proj * Matrix4::look_at(point.position, point.position + Vector3(-1.0,  0.0,  0.0), Vector3(0.0, -1.0,  0.0)),
        m_shadow_proj * Matrix4::look_at(point.position, point.position + Vector3( 0.0,  1.0,  0.0), Vector3(0.0,  0.0,  1.0)),
        m_shadow_proj * Matrix4::look_at(point.position, point.position + Vector3( 0.0, -1.0,  0.0), Vector3(0.0,  0.0, -1.0)),
        m_shadow_proj * Matrix4::look_at(point.position, point.position + Vector3( 0.0,  0.0,  1.0), Vector3(0.0, -1.0,  0.0)),
        m_shadow_proj * Matrix4::look_at(point.position, point.position + Vector3( 0.0,  0.0, -1.0), Vector3(0.0, -1.0,  0.0))
    };
}                                                 

    
    void AdvancedLightningScene::setup_point_shadows() const
    {
        m_material->use();
        m_material->set_uniform("far_plane", 25.0f);
        m_material->bind_depth_cubemap_array(3, &m_omnidirectional_shadow_map->get_depth_attachment(), m_sampler.get());
        
    }
} // namespace game