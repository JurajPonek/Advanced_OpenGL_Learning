#include "advanced_lightning_scene.hpp"
#include "framebuffer.hpp"
#include "imgui.h"
#include "opengl.hpp"
#include "texture.hpp"
#include <gl/gl.h>
#include <memory>

namespace
{
    bool g_enable_msaa = true;
}


namespace game
{
    AdvancedLightningScene::AdvancedLightningScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer, MeshLoader* mesh_loader)
        : m_entities{}, m_camera{camera}, m_renderer{renderer},  m_mesh_loader{mesh_loader}
    {
        FramebufferSpecification spec{};
        spec.width = window->get_width();
        spec.height = window->get_height();
        spec.samples = 8;
        spec.attachments = {TextureUsage::COLORATTACHMENT, TextureUsage::DEPTHATTACHMENT};
        m_msaa_fbo = std::make_unique<FrameBuffer>(spec);
        spec.samples = 1;
        m_post_process_fbo = std::make_unique<FrameBuffer>(spec);

        m_texture = std::make_unique<Texture>(resource_loader.load_binary("container2.png"), 500, 500);
        m_sampler = std::make_unique<Sampler>();
        const Texture* textures[]{m_texture.get()};
        const Sampler* samplers[]{m_sampler.get()};
        const auto tex_samp = std::views::zip(textures, samplers) | std::ranges::to<std::vector>();

        const auto vertex_shader =
            Shader(resource_loader.load_string("shaders/default_vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader =
            Shader(resource_loader.load_string("shaders/default_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto post_process_vert =
            Shader(resource_loader.load_string("shaders/post_process_vert.glsl"), game::ShaderType::VERTEX);
        const auto post_process_frag =
            Shader(resource_loader.load_string("shaders/post_process_grayscale.glsl"), game::ShaderType::FRAGMENT);
        m_material = std::make_unique<Material>(vertex_shader, fragment_shader);
        m_post_process_material = std::make_unique<Material>(post_process_vert, post_process_frag);
        m_cube = std::make_unique<Mesh>(m_mesh_loader->cube());
        m_plane = std::make_unique<Mesh>(m_mesh_loader->plane());
        m_sphere = std::make_unique<Mesh>(m_mesh_loader->sphere());
        m_entities.emplace_back(m_cube.get(), m_material.get(), Vector3{1.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp);
        m_entities.emplace_back(m_plane.get(), m_material.get(), Vector3{0.0f, 0.0f, 0.0f}, Vector3{10.0f, 1.0f, 10.0f}, tex_samp);
        m_entities.emplace_back(m_sphere.get(), m_material.get(), Vector3{5.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp);
    }
    void AdvancedLightningScene::on_render()
    {
        if (g_enable_msaa)
        {
            m_msaa_fbo->bind();
            ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::glEnable(GL_DEPTH_TEST);
            m_renderer->set_camera(m_camera);
            for (const auto& entity : m_entities)
            {
                m_renderer->draw_mesh(entity.get_mesh(), entity.get_material(), entity.get_model_matrix(),
                                      entity.get_textures());
            }
            m_msaa_fbo->unbind();
            ::glBlitNamedFramebuffer(m_msaa_fbo->get_native_handle(), m_post_process_fbo->get_native_handle(), 0, 0,
                                     m_msaa_fbo->get_width(), m_msaa_fbo->get_height(), 0, 0,
                                     m_post_process_fbo->get_width(), m_post_process_fbo->get_height(),
                                     GL_COLOR_BUFFER_BIT, GL_NEAREST);

            ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::glDisable(GL_DEPTH_TEST);
            m_renderer->draw_post_process_texture(m_post_process_material.get(), m_sampler.get(),
                                                  m_post_process_fbo.get());
        }
        else
        {
            m_post_process_fbo->bind();
            ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::glEnable(GL_DEPTH_TEST);
            m_renderer->set_camera(m_camera);
            for (const auto& entity : m_entities)
            {
                m_renderer->draw_mesh(entity.get_mesh(), entity.get_material(), entity.get_model_matrix(),
                                      entity.get_textures());
            }
            m_post_process_fbo->unbind();
            ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::glDisable(GL_DEPTH_TEST);
            m_renderer->draw_post_process_texture(m_post_process_material.get(), m_sampler.get(),
                                                  m_post_process_fbo.get());
        }
        
    }
    void AdvancedLightningScene::on_imgui_render()
    {
        ::ImGui::Checkbox("MSAA", &g_enable_msaa);
    }
    void AdvancedLightningScene::on_attach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
    void AdvancedLightningScene::on_detach()
    {

    }
}