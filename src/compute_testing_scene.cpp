#include "compute_testing_scene.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "opengl.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "shader.hpp"
#include "src/buffer_writer.hpp"
#include "src/window.hpp"
#include "texture.hpp"
#include "vendor/opengl/glext.h"
#include <array>
#include <cstdint>
#include <gl/gl.h>
#include <memory>
#include <print>
#include <vector>
#include <ranges>



namespace game
{
    ComputeTestingScene::ComputeTestingScene(const ResourceLoader& loader, Window* window, Camera* camera,
                                             Renderer* renderer)
        : m_camera(camera), m_window(window), m_renderer(renderer),m_test_buffer{static_cast<uint32_t>(m_window->get_width() * m_window->get_height() * sizeof(float))}
    {
        FramebufferSpecification spec;
        spec.width = window->get_width();
        spec.height = window->get_height();
        spec.type = TextureType::TEXTURE2D;
        spec.attachments = {TextureFormat::RGBA8};
        m_fbo = std::make_unique<FrameBuffer>(spec);
        const auto compute_shader = Shader(loader.load_string("shaders/test_compute.glsl"), game::ShaderType::COMPUTE);
        const auto default_vert_shader = Shader(loader.load_string("shaders/full_screen_quad_vert.glsl"), game::ShaderType::VERTEX);
        const auto default_frag_shader = Shader(loader.load_string("shaders/default_frag.glsl"), game::ShaderType::FRAGMENT);
        m_compute_material = std::make_unique<Material>(compute_shader);
        m_default_material = std::make_unique<Material>(default_vert_shader, default_frag_shader);
        m_sampler = std::make_unique<Sampler>();
        m_test_data.resize(m_window->get_width() * m_window->get_height());
        m_test_texture =
            std::make_unique<Texture>(loader.load_binary("brickwall.jpg"));
    }
    void ComputeTestingScene::on_render()
    {
        // ::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // ::glEnable(GL_DEPTH_TEST);
        
        m_compute_material->use();
        m_compute_material->bind_texture(0, m_test_texture.get(), m_sampler.get());
        ::glBindImageTexture(1, m_fbo->get_color_attachment(0).get_native_handle(), 0, GL_FALSE, 0, GL_WRITE_ONLY,
                             GL_RGBA8);
        GLuint local_size_x = 16;
        GLuint local_size_y = 16;
        GLuint group_size_x = (m_window->get_width() + local_size_x - 1) / local_size_x;
        GLuint group_size_y = (m_window->get_height() + local_size_y - 1) / local_size_y;
        ::glDispatchCompute(group_size_x, group_size_y, 1);
        ::glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_fbo->bind();
        m_default_material->use();
        m_default_material->bind_texture(0, &m_fbo->get_color_attachment(0), m_sampler.get());
        m_fbo->unbind();
        m_renderer->draw_fullscreen_quad();






    }




} // namespace game