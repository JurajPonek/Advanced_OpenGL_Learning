#include "PBR_scene.hpp"
#include "buffer_writer.hpp"
#include "color.hpp"
#include "cubemap.hpp"
#include "error.hpp"
#include "framebuffer.hpp"
#include "imgui.h"
#include "material.hpp"
#include "matrix4.hpp"
#include "opengl.hpp"
#include "sampler.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include "vendor/opengl/glext.h"
#include <ImGuizmo.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <gl/gl.h>
#include <memory>
#include <numbers>
#include <random>
#include <ranges>
#include <span>
#include <vector>


namespace
{
    bool g_use_height_map = true;
    bool g_enable_hdr = true;
    bool g_enable_ssao = true;
    float g_height_map_scale = 0.015;
    float g_ssao_power = 1;
    float g_gamma = 2.2f;
    game::Vector3 g_pbr_test_albedo = {1.0f, 0.0f, 0.0f};
    float g_pbr_test_roughness = 0.0f;
    float g_pbr_test_metallic = 0.0f;
    float g_pbr_test_ao = 0.0f;
    struct PointLightBuffer
    {
        alignas(16) game::Vector3 position{};
        alignas(16) game::Color color{};
        float radius{};
        float intensity{};
        int shadow_map_index;
    };
    float lerp(float a, float b, float t) { return a + (b - a) * t; }

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
    static constexpr int MAX_POINT_LIGHTS = 100;
    static constexpr int MAX_POINT_LIGHTS_CASTING_SHADOWS = 3;
    static constexpr int SSAO_NOISE_SIZE = 16;
    PBRScene::PBRScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer,
                       MeshLoader* mesh_loader)
        : m_entities{}, m_camera{camera}, m_renderer{renderer}, m_mesh_loader{mesh_loader}, m_points{},
          m_light_buffer{10240u}, m_directional{{0.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}, m_ambient{0.3f, 0.3f, 0.3f}
    {


        FramebufferSpecification spec{};
        spec.width = window->get_width();
        spec.height = window->get_height();
        spec.type = TextureType::TEXTURE2D;
        spec.samples = 1;
        spec.attachments = {TextureFormat::RGBA8, TextureFormat::RGBA16F, TextureFormat::RGBA8, TextureFormat::Depth32F};
        m_g_buffer = std::make_unique<FrameBuffer>(spec);

        spec.attachments = {TextureFormat::RED};
        m_ssao_fbo = std::make_unique<FrameBuffer>(spec);

        m_ssao_blur_fbo = std::make_unique<FrameBuffer>(spec);

        spec.attachments = {TextureFormat::RGBA16F, TextureFormat::Depth32F};
        m_post_process_fbo = std::make_unique<FrameBuffer>(spec);

        spec.attachments = {TextureFormat::RGBA8};
        m_debug_fbo = std::make_unique<FrameBuffer>(spec);

        spec.attachments = {TextureFormat::Depth32F};
        spec.width = SHADOW_MAP_WIDTH;
        spec.height = SHADOW_MAP_HEIGHT;
        m_shadow_map = std::make_unique<FrameBuffer>(spec);

        spec.attachments = {TextureFormat::Depth32F};
        spec.max_lights = MAX_POINT_LIGHTS_CASTING_SHADOWS;
        spec.type = TextureType::DEPTHCUBEMAP;
        m_omnidirectional_shadow_map = std::make_unique<FrameBuffer>(spec);

        m_shadow_proj =
            Matrix4::perspective(std::numbers::pi_v<float> / 2.0f, m_omnidirectional_shadow_map->get_width(),
                                 m_omnidirectional_shadow_map->get_height(), 1.0f, 25.0f);

        m_points.emplace_back(
            PointLight{{0.0f, 5.0f, 1.0f}, {0.5f, 0.5f, 0.5f}, 25.0f, 30.0f, static_cast<int>(m_points.size())});




        m_sampler = std::make_unique<Sampler>();
        m_shadow_map_sampler = std::make_unique<Sampler>(SamplerUsage::SHADOWMAP);

        m_sloppy_mortar_albedo = std::make_unique<Texture>(
            resource_loader.load_binary("PBR/sloppy_mortar/sloppy-mortar-stone-wall_albedo.png"), TextureFormat::SRGBA);
        m_sloppy_mortar_normal = std::make_unique<Texture>(
            resource_loader.load_binary("PBR/sloppy_mortar/sloppy-mortar-stone-wall_normal-ogl.png"));
        m_sloppy_mortar_metallic = std::make_unique<Texture>(
            resource_loader.load_binary("PBR/sloppy_mortar/sloppy-mortar-stone-wall_metallic.png"));
        m_sloppy_mortar_roughness = std::make_unique<Texture>(
            resource_loader.load_binary("PBR/sloppy_mortar/sloppy-mortar-stone-wall_roughness.png"));
        m_sloppy_mortar_height = std::make_unique<Texture>(
            resource_loader.load_binary("PBR/sloppy_mortar/sloppy-mortar-stone-wall_height.png"));
        m_sloppy_mortar_ao =
            std::make_unique<Texture>(resource_loader.load_binary("PBR/sloppy_mortar/sloppy-mortar-stone-wall_ao.png"));

        m_bare_wood_albedo = std::make_unique<Texture>(resource_loader.load_binary("PBR/bare_wood/bare-wood1_albedo.png"), TextureFormat::SRGBA);
        m_bare_wood_normal = std::make_unique<Texture>(resource_loader.load_binary("PBR/bare_wood/bare-wood1_normal-ogl.png"));
        m_bare_wood_metallic = std::make_unique<Texture>(resource_loader.load_binary("PBR/bare_wood/bare-wood1_metallic.png"));
        m_bare_wood_roughness = std::make_unique<Texture>(resource_loader.load_binary("PBR/bare_wood/bare-wood1_roughness.png"));
        m_bare_wood_height = std::make_unique<Texture>(resource_loader.load_binary("PBR/bare_wood/bare-wood1_height.png"));
        m_bare_wood_ao = std::make_unique<Texture>(resource_loader.load_binary("PBR/bare_wood/bare-wood1_ao.png"));
        TextureSpecification default_map_spec;
        default_map_spec.default_normal_map_texture = true;
        default_map_spec.type = TextureType::TEXTURE2D;
        m_default_normal_map_texture = std::make_unique<Texture>(default_map_spec);

        default_map_spec.default_normal_map_texture = false;
        default_map_spec.default_height_map_texture = true;
        default_map_spec.type = TextureType::TEXTURE2D;
        m_default_height_map_texture = std::make_unique<Texture>(default_map_spec);


        const Sampler* samplers[] = {m_sampler.get(), m_sampler.get(), m_sampler.get(), m_sampler.get(), m_sampler.get(), m_sampler.get()};
        const Texture* textures[] = {m_sloppy_mortar_albedo.get(),   m_sloppy_mortar_normal.get(),
                                     m_sloppy_mortar_height.get(),   m_sloppy_mortar_ao.get(),
                                     m_sloppy_mortar_metallic.get(), m_sloppy_mortar_roughness.get()};
        const Texture* textures1[] = { m_default_normal_map_texture.get(),   m_default_height_map_texture.get()};
        const Texture* textures2[] = {m_bare_wood_albedo.get(),   m_bare_wood_normal.get(),
                                      m_bare_wood_height.get(),   m_bare_wood_ao.get(),
                                      m_bare_wood_metallic.get(), m_bare_wood_roughness.get()};

        const auto tex_samp1 = std::views::zip(textures, samplers) | std::ranges::to<std::vector>();
        const auto tex_samp2 = std::views::zip(textures1, samplers) | std::ranges::to<std::vector>();
        const auto tex_samp3 = std::views::zip(textures2, samplers) | std::ranges::to<std::vector>();

        const auto vertex_shader =
            Shader(resource_loader.load_string("shaders/PBR_vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader =
            Shader(resource_loader.load_string("shaders/PBR_frag.glsl"), game::ShaderType::FRAGMENT);

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

        const auto g_pass_vert =
            Shader(resource_loader.load_string("shaders/PBR_g_pass_vert.glsl"), game::ShaderType::VERTEX);
        const auto g_pass_frag =
            Shader(resource_loader.load_string("shaders/PBR_g_pass_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto full_screen_quad_vert =
            Shader(resource_loader.load_string("shaders/full_screen_quad_vert.glsl"), game::ShaderType::VERTEX);
        const auto ssao_frag =
            Shader(resource_loader.load_string("shaders/ssao_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto ssao_blur_frag =
            Shader(resource_loader.load_string("shaders/ssao_blur_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto debug_view_frag =
            Shader(resource_loader.load_string("shaders/debug_view_frag.glsl"), game::ShaderType::FRAGMENT);
        const auto pbr_test_frag =
            Shader(resource_loader.load_string("shaders/PBR_g_pass_test_frag.glsl"), game::ShaderType::FRAGMENT);
        const auto background_vert =
            Shader(resource_loader.load_string("shaders/background_vert.glsl"), game::ShaderType::VERTEX);
        const auto background_frag =
            Shader(resource_loader.load_string("shaders/background_frag.glsl"), game::ShaderType::FRAGMENT);


        m_material = std::make_unique<Material>(vertex_shader, fragment_shader);
        m_post_process_material = std::make_unique<Material>(post_process_vert, post_process_frag);
        m_shadow_map_material = std::make_unique<Material>(shadow_map_vert, shadow_map_frag);
        m_point_shadows_material =
            std::make_unique<Material>(point_shadows_vert, point_shadows_geo, point_shadows_frag);
        m_g_buffer_material = std::make_unique<Material>(g_pass_vert, g_pass_frag);
        m_ssao_material = std::make_unique<Material>(full_screen_quad_vert, ssao_frag);
        m_ssao_blur_material = std::make_unique<Material>(full_screen_quad_vert, ssao_blur_frag);
        m_debug_view_material = std::make_unique<Material>(full_screen_quad_vert, debug_view_frag);
        m_pbr_test_material = std::make_unique<Material>(g_pass_vert, pbr_test_frag);
        m_background_material = std::make_unique<Material>(background_vert, background_frag);

        m_sphere = std::make_unique<Mesh>(m_mesh_loader->sphere());
        m_plane = std::make_unique<Mesh>(m_mesh_loader->plane());

        m_entities.emplace_back(m_sphere.get(), m_g_buffer_material.get(), Vector3{5.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp1, true);
        m_entities.emplace_back(m_sphere.get(), m_pbr_test_material.get(), Vector3{1.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp2);
        m_entities.emplace_back(m_plane.get(), m_g_buffer_material.get(), Vector3{1.0f, -1.0f, 1.0f}, Vector3{10.0f, 1.0f, 10.0f},
                                tex_samp3);

        m_env_map = std::make_unique<CubeMap>("HDR_Maps/grasslands_sunset_4k.hdr", resource_loader);
        m_irradiance_map = std::make_unique<CubeMap>(CubeMap::generate_irradiance_map(m_env_map.get(), resource_loader));



        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::uniform_real_distribution dist(-1.0f, 1.0f);
        m_ssao_kernel.resize(SSAO_KERNEL_SIZE);
        for (auto i{0}; i < SSAO_KERNEL_SIZE; ++i)
        {
            Vector3 random_sample = Vector3::normalize({dist(gen), dist(gen), (dist(gen) + 1.0f) / 2.0f});
            random_sample *= ((dist(gen) + 1.0f) / 2.0f);
            float scale = float(i) / float(SSAO_KERNEL_SIZE);
            scale = lerp(0.1f, 1.0f, scale * scale);
            random_sample *= scale;
            m_ssao_kernel[i] = random_sample;
        }
        std::vector<Vector3> ssao_noise{};
        ssao_noise.resize(SSAO_NOISE_SIZE);
        for (auto i{0}; i < SSAO_NOISE_SIZE; ++i)
        {
            ssao_noise[i] = Vector3::normalize({dist(gen), dist(gen), 0.0f});
        }
        TextureSpecification ssao_noise_texture_spec;
        ssao_noise_texture_spec.width = 4;
        ssao_noise_texture_spec.height = 4;
        ssao_noise_texture_spec.format = TextureFormat::RGBA16F;
        ssao_noise_texture_spec.texture_wrapping = TextureWrappingMode::REPEAT;
        ssao_noise_texture_spec.filter_mode = TextureFilterMode::NEAREST;
        m_ssao_noise_texture = std::make_unique<Texture>(ssao_noise, ssao_noise_texture_spec);
    }
    void PBRScene::on_render()
    {
        Matrix4 light_space_matrix = calculate_light_space_matrix();
        execute_shadow_pass(light_space_matrix);
        execute_g_pass();
        execute_ssao_pass();
        execute_lighting_pass(light_space_matrix);
        draw_background();
        execute_post_process_pass();
    }
    void PBRScene::on_imgui_render()
    {
        ::ImGui::Checkbox("HDR", &g_enable_hdr);
        ::ImGui::Checkbox("SSAO", &g_enable_ssao);
        ::ImGui::Checkbox("USE_HEIGHT_MAPS", &g_use_height_map);
        ::ImGui::SliderFloat("Height_scale", &g_height_map_scale, 0.0f, 1.0f);
        ::ImGui::SliderFloat("Gamma", &g_gamma, 0.0f, 5.0f);
        ::ImGui::SliderFloat("SSAO Power", &g_ssao_power, 1.0f, 10.0f);
        ::ImGuiIO& io = ImGui::GetIO();
        ::ImGuizmo::SetOrthographic(false);
        ::ImGuizmo::BeginFrame();
        ::ImGuizmo::Enable(true);
        ::ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        static auto selected_point = 0;
        if (ImGui::Button("Add light"))
        {
            const auto& last = m_points.back();
            ensure(m_points.size() < MAX_POINT_LIGHTS, "Reached point lights limit");
            int point_shadow_index = MAX_POINT_LIGHTS_CASTING_SHADOWS <= m_points.size() ? -1 : m_points.size();
            m_points.emplace_back(
                PointLight{last.position, last.color, last.radius, last.intensity, point_shadow_index});
            selected_point = m_points.size() - 1u;
        }
        if (::ImGui::CollapsingHeader("PBR Test"))
        {
            ::ImGui::ColorPicker3("alebedo", &g_pbr_test_albedo.x);
            ::ImGui::SliderFloat("roughness", &g_pbr_test_roughness, 0.0f, 1.0f);
            ::ImGui::SliderFloat("metallic", &g_pbr_test_metallic, 0.0f, 1.0f);
            ::ImGui::SliderFloat("AO", &g_pbr_test_ao, 0.0f, 1.0f);
        }
        if (::ImGui::CollapsingHeader("ambient"))
        {
            ::ImGui::ColorPicker3("ambient color", &m_ambient.r);
        }
        if (::ImGui::CollapsingHeader("directional"))
        {
            ::ImGui::SliderFloat3("Direction", &m_directional.direction.x, -10.0f, 10.0f);
            ::ImGui::ColorPicker3("directional color", &m_directional.color.r);
        }
        for (const auto&& [index, point] : m_points | std::views::enumerate)
        {
            const auto header_name = std::format("pointlight {}", index);
            const auto picker_name = std::format("color {}", index);
            const auto shininess_name = std::format("shininess {}", index);
            const auto radius_name = std::format("radius {}", index);
            const auto intensity_name = std::format("intensity {}", index);
            if (::ImGui::CollapsingHeader(header_name.c_str()))
            {
                if (::ImGui::ColorEdit3(picker_name.c_str(), &point.color.r))
                {
                    selected_point = index;
                }
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
        ImGui::Begin("Shadow map");
        ::GLuint texture_id = m_shadow_map->get_depth_attachment().get_native_handle();
        ImVec2 image_size = ImVec2(320.0f, 180.0f);
        ImTextureID imgui_texture_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id));
        ImGui::Image(imgui_texture_id, image_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        ImGui::End();
        ImGui::Begin("Normals");
        texture_id = m_g_buffer->get_color_attachment(1).get_native_handle();
        imgui_texture_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id));
        ImGui::Image(imgui_texture_id, image_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        ImGui::End();
        ImGui::Begin("Albedo");
        texture_id = m_g_buffer->get_color_attachment().get_native_handle();
        imgui_texture_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id));
        ImGui::Image(imgui_texture_id, image_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        ImGui::End();
        debug_draw(m_ssao_fbo->get_color_attachment());
        ImGui::Begin("AO");
        texture_id = m_debug_fbo->get_color_attachment().get_native_handle();
        imgui_texture_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id));
        ImGui::Image(imgui_texture_id, image_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        // ImGui::Text("FBO size: %dx%d", m_fbo->get_width(), m_fbo->get_height());


        ImGui::End();
    }
    void PBRScene::on_attach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // bez tohto su vidno hrany cubemapy
    }
    void PBRScene::on_detach() { ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0); }
    void PBRScene::setup_lights() const
    {
        LightBuffer light_buffer{m_ambient, m_directional.direction, m_directional.color,
                                 static_cast<int>(m_points.size())};
        BufferWriter writer{m_light_buffer};
        writer.write(light_buffer);
        for (const auto& point : m_points)
        {
            PointLightBuffer point_buffer = {point.position, point.color,
                                             point.radius,   point.intensity, point.shadow_map_index};
            writer.write(point_buffer);
        }
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_light_buffer.get_native_handle());
    }
    void PBRScene::setup_shadows(const Matrix4& lightSpaceMatrix) const
    {
        m_material->use();
        m_material->set_uniform("light_space_matrix", lightSpaceMatrix);
        m_material->bind_texture(3, &m_shadow_map->get_depth_attachment(), m_shadow_map_sampler.get());
    }
    std::array<Matrix4, 6> PBRScene::calculate_shadow_transformations(const PointLight& point) const
    {
        return {m_shadow_proj *
                    Matrix4::look_at(point.position, point.position + Vector3(1.0, 0.0, 0.0), Vector3(0.0, -1.0, 0.0)),
                m_shadow_proj *
                    Matrix4::look_at(point.position, point.position + Vector3(-1.0, 0.0, 0.0), Vector3(0.0, -1.0, 0.0)),
                m_shadow_proj *
                    Matrix4::look_at(point.position, point.position + Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, 1.0)),
                m_shadow_proj *
                    Matrix4::look_at(point.position, point.position + Vector3(0.0, -1.0, 0.0), Vector3(0.0, 0.0, -1.0)),
                m_shadow_proj *
                    Matrix4::look_at(point.position, point.position + Vector3(0.0, 0.0, 1.0), Vector3(0.0, -1.0, 0.0)),
                m_shadow_proj * Matrix4::look_at(point.position, point.position + Vector3(0.0, 0.0, -1.0),
                                                 Vector3(0.0, -1.0, 0.0))};
    }


    void PBRScene::setup_point_shadows() const
    {
        m_material->use();
        m_material->set_uniform("far_plane", 25.0f);
        m_material->bind_depth_cubemap_array(4, &m_omnidirectional_shadow_map->get_depth_attachment(), m_sampler.get());
    }

    void PBRScene::setup_other_uniforms() const
    {
        m_g_buffer_material->set_uniform("use_height_map", g_use_height_map);
    }
    void PBRScene::execute_g_pass() const
    {
        m_g_buffer->bind();
        ::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glEnable(GL_DEPTH_TEST);
        m_renderer->set_camera(m_camera);
        //setup_other_uniforms();
        for (const auto& entity : m_entities)
        {
            const auto* material = entity.get_material();
            
            material->set_uniform("height_scale", entity.has_height_map() ? g_height_map_scale : 0.0f);
            material->set_uniform("use_height_map", g_use_height_map);

            if (material == m_pbr_test_material.get())
            {
                material->set_uniform("in_albedo", g_pbr_test_albedo);
                material->set_uniform("in_ao", g_pbr_test_ao);
                material->set_uniform("in_roughness", g_pbr_test_roughness);
                material->set_uniform("in_metallic", g_pbr_test_metallic);
            }

            m_renderer->draw_mesh(entity.get_mesh(), material, entity.get_model_matrix(),
                                  entity.get_textures());
        }
        m_g_buffer->unbind();
    }
    void PBRScene::execute_lighting_pass(const Matrix4& light_space_matrix) const
    {
        m_post_process_fbo->bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glDisable(GL_DEPTH_TEST);
        m_renderer->set_camera(m_camera);
        setup_lights();
        setup_shadows(light_space_matrix);
        setup_point_shadows();
        setup_textures_from_g_buffer();
        m_material->bind_texture(5, &m_ssao_blur_fbo->get_color_attachment(), m_sampler.get());
        m_material->bind_cubemap(m_irradiance_map.get(), m_sampler.get(), 7);
        m_material->set_uniform("ssao", g_enable_ssao);
        Matrix4 view_proj_inverse =
            Matrix4::inverse(m_camera->get_projection_as_matrix()  * m_camera->get_view_as_matrix());
        m_material->set_uniform("inv_view_proj", view_proj_inverse);
        m_renderer->draw_fullscreen_quad();
        m_post_process_fbo->unbind();
    }
    void PBRScene::execute_post_process_pass() const
    {
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glDisable(GL_DEPTH_TEST);
        m_post_process_material->set_uniform("gamma", g_gamma);
        m_post_process_material->set_uniform("enable_hdr", g_enable_hdr);
        m_renderer->draw_post_process_texture(m_post_process_material.get(), m_sampler.get(), m_post_process_fbo.get());
    }
    void PBRScene::execute_shadow_pass(const Matrix4& light_space_matrix) const
    {
        ::glEnable(GL_DEPTH_TEST);
        m_shadow_map->bind();
        ::glClear(GL_DEPTH_BUFFER_BIT);
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
    }
    Matrix4 PBRScene::calculate_light_space_matrix() const
    {
        Vector3 light_dir = Vector3::normalize(m_directional.direction);
        Vector3 scene_center = {0.0f, 0.0f, 0.0f};
        float shadow_distance = 10.0f;
        Vector3 light_pos = scene_center - light_dir * shadow_distance;
        float near_plane = 1.0f, far_plane = 50.0f;
        Matrix4 light_view = Matrix4::look_at(light_pos, scene_center, {0.0f, 1.0f, 0.0f});
        Matrix4 light_projection = Matrix4::orthographic(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);
        return light_projection * light_view;
    }

    void PBRScene::setup_textures_from_g_buffer() const
    {
        m_material->bind_texture(0, &m_g_buffer->get_color_attachment(0), m_sampler.get());
        m_material->bind_texture(1, &m_g_buffer->get_color_attachment(1), m_sampler.get());
        m_material->bind_texture(2, &m_g_buffer->get_color_attachment(2), m_sampler.get());
        m_material->bind_texture(6, &m_g_buffer->get_depth_attachment(), m_sampler.get());
    }
    void PBRScene::execute_ssao_pass() const
    {
        m_ssao_fbo->bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glDisable(GL_DEPTH_TEST);
        m_renderer->set_camera(m_camera);
        m_ssao_material->use();
        m_ssao_material->bind_texture(0, &m_g_buffer->get_color_attachment(1), m_sampler.get());
        m_ssao_material->bind_texture(1, &m_g_buffer->get_depth_attachment(), m_sampler.get());
        m_ssao_material->bind_texture(2, m_ssao_noise_texture.get(), m_sampler.get());
        m_ssao_material->set_uniform("samples[0]", m_ssao_kernel);
        Matrix4 inv_proj = Matrix4::inverse(m_camera->get_projection_as_matrix());
        m_ssao_material->set_uniform("inverse_proj", inv_proj);
        m_ssao_material->set_uniform("power", g_ssao_power);
        m_renderer->draw_fullscreen_quad();
        m_ssao_fbo->unbind();
        m_ssao_blur_fbo->bind();
        m_ssao_blur_material->use();
        m_ssao_blur_material->bind_texture(0, &m_ssao_fbo->get_color_attachment(), m_sampler.get());
        m_renderer->draw_fullscreen_quad();
        m_ssao_blur_fbo->unbind();
    }

    void PBRScene::debug_draw(const Texture& attachment) const
    {
        m_debug_fbo->bind();
        m_debug_view_material->use();
        m_debug_view_material->bind_texture(0, &attachment, m_sampler.get());
        m_renderer->draw_fullscreen_quad();
        m_debug_fbo->unbind();
    }
    void PBRScene::draw_background() const
    {
        m_post_process_fbo->bind();

        ::glBlitNamedFramebuffer(m_g_buffer->get_native_handle(),    
                                 m_post_process_fbo->get_native_handle(),
                                 0, 0, m_g_buffer->get_width(), m_g_buffer->get_height(), 0, 0,
                                 m_post_process_fbo->get_width(), m_post_process_fbo->get_height(), GL_DEPTH_BUFFER_BIT,
                                 GL_NEAREST);

        ::glEnable(GL_DEPTH_TEST);
        ::glDepthFunc(GL_LEQUAL);
        ::glDepthMask(GL_FALSE); 

        m_background_material->use();

        Matrix4 view = m_camera->get_view_as_matrix();
        view[12] = 0.0f;
        view[13] = 0.0f;
        view[14] = 0.0f;

        Matrix4 inv_view_proj = Matrix4::inverse(m_camera->get_projection_as_matrix() * view);
        m_background_material->set_uniform("u_InvViewProj", inv_view_proj);

        m_background_material->bind_cubemap(m_env_map.get(), m_sampler.get());

        m_renderer->draw_fullscreen_quad();
        ::glDepthMask(GL_TRUE);
        ::glDepthFunc(GL_LESS);
        m_post_process_fbo->unbind();
    }


} // namespace game