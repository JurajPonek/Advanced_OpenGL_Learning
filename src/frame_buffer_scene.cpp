#include "frame_buffer_scene.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "opengl.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "texture.hpp"
#include "window.hpp"
#include <gl/gl.h>
#include <memory>

namespace game
{
    FrameBufferScene::FrameBufferScene(ResourceLoader& resource_loader, Window* window, Camera* camera,
                                       Renderer* renderer)
        : m_entities{}, 
          m_camera{camera}, m_renderer{renderer},
          m_fbo{window->get_width(), window->get_height()}

    {
        MeshLoader mesh_loader{resource_loader};
        m_texture = std::make_unique<Texture>(resource_loader.load_binary("container2.png"), 500, 500);
        m_sampler = std::make_unique<Sampler>();

        const Texture* textures[]{m_texture.get()};
        const Sampler* samplers[]{m_sampler.get()};
        const auto tex_samp = std::views::zip(textures, samplers) | std::ranges::to<std::vector>();

        const auto vertex_shader = Shader(resource_loader.load_string("shaders/frame_buffer_vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader = Shader(resource_loader.load_string("shaders/frame_buffer_frag.glsl"), game::ShaderType::FRAGMENT);

        const auto post_process_vert =
            Shader(resource_loader.load_string("shaders/post_process_vert.glsl"), game::ShaderType::VERTEX);
        const auto post_process_frag =
            Shader(resource_loader.load_string("shaders/post_process_frag.glsl"), game::ShaderType::FRAGMENT);


        m_material = std::make_unique<Material>(vertex_shader, fragment_shader);
        m_post_process_material = std::make_unique<Material>(post_process_vert, post_process_frag);
        m_cube = std::make_unique<Mesh>(mesh_loader.cube());
        m_plane = std::make_unique<Mesh>(mesh_loader.plane());
        m_sphere = std::make_unique<Mesh>(mesh_loader.sphere());

        m_entities.emplace_back(m_cube.get(), m_material.get(), Vector3{1.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp);
        m_entities.emplace_back(m_plane.get(), m_material.get(), Vector3{0.0f, 0.0f, 0.0f}, Vector3{10.0f, 1.0f, 10.0f}, tex_samp);
        m_entities.emplace_back(m_sphere.get(), m_material.get(), Vector3{5.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp);
    }
    void FrameBufferScene::on_render()
    {
        m_fbo.bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glEnable(GL_DEPTH_TEST);
        m_renderer->set_camera(m_camera);
        for (const auto& entity : m_entities)
        {
            m_renderer->draw_mesh(entity.get_mesh(), entity.get_material(), entity.get_model_matrix(),
                                  entity.get_textures());
        }
        m_fbo.unbind();
        ::glClear(GL_COLOR_BUFFER_BIT);
        ::glDisable(GL_DEPTH_TEST);
        m_renderer->draw_post_process_texture(m_post_process_material.get(), m_sampler.get(), m_fbo); 
    }
    void FrameBufferScene::on_imgui_render()
    {
        ImGui::Begin("Framebuffer Preview");

        ::GLuint texture_id = m_fbo.get_color_attachment().get_native_handle();

        ImVec2 image_size = ImVec2(320.0f, 180.0f);
        ImTextureID imgui_texture_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texture_id));

        ImGui::Image(imgui_texture_id, image_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        ImGui::Text("FBO size: %dx%d", m_fbo.get_width(), m_fbo.get_height());

        ImGui::End();
    }
    void FrameBufferScene::on_attach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

    }
    void FrameBufferScene::on_detach()
    {

    }
}