#include "renderer.hpp"
#include "buffer_writer.hpp"
#include "color.hpp"
#include "cubemap.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "opengl.hpp"
#include "matrix4.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include <gl/gl.h>
#include <ranges>
#include <span>
#include <tuple>
#include <vector>
#include "camera.hpp"
#include "opengl.hpp"
#include "vendor/opengl/glext.h"


namespace game
{
    Renderer::Renderer(MeshLoader& mesh_loader, ResourceLoader& resource_loader) 
    : m_camera_buffer {sizeof(Matrix4) * 2 + sizeof(Vector3)}, m_post_process_vao{0u, [](auto vao){::glDeleteVertexArrays(1, &vao);}}, m_skybox(mesh_loader.cube()), m_skybox_material{setup_skybox_material(resource_loader)}
    {
        ::glGenVertexArrays(1, &m_post_process_vao);

    }
    void Renderer::set_camera(const Camera* camera)
    {
        BufferWriter writer{m_camera_buffer};
        writer.write(camera->get_view());
        writer.write(camera->get_projection());
        writer.write(camera->get_position());
        ::glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_camera_buffer.get_native_handle());
    }
    void Renderer::draw_mesh(const Mesh* mesh, const Material* material, const Matrix4& transform,
                             std::span<const std::tuple<const Texture*, const Sampler*>> textures) const
    {
        material->use();
        material->set_uniform("model", transform);
        material->bind_textures(textures);
        mesh->bind();
        ::glDrawElements(GL_TRIANGLES, mesh->get_index_count(), GL_UNSIGNED_INT,
                         reinterpret_cast<void*>(mesh->get_index_offset()));
        mesh->unbind();
    }
    void Renderer::draw_post_process_texture(const Material* material, const Sampler* sampler, const FrameBuffer& fbo)
    {
        material->use();
        material->bind_texture(0, &fbo.get_color_attachment(), sampler);
        ::glBindVertexArray(m_post_process_vao);
        ::glDrawArrays(GL_TRIANGLES, 0, 3);
        ::glBindVertexArray(0);
    }
    Material Renderer::setup_skybox_material(ResourceLoader& resource_loader) const
    {
        const auto vertex_shader =
            Shader(resource_loader.load_string("shaders/skybox_vert.glsl"), game::ShaderType::VERTEX);
        const auto fragment_shader =
            Shader(resource_loader.load_string("shaders/skybox_frag.glsl"), game::ShaderType::FRAGMENT);
        return {vertex_shader, fragment_shader};

    }
    void Renderer::draw_skybox(CubeMap* cubemap, Sampler* sampler) const
    {
        ::glDisable(GL_CULL_FACE);
        ::glDepthFunc(GL_LEQUAL);  
        m_skybox_material.use();
        m_skybox_material.bind_cubemap(cubemap, sampler);
        m_skybox.bind();
        ::glDrawElements(GL_TRIANGLES, m_skybox.get_index_count(), GL_UNSIGNED_INT, reinterpret_cast<void*>(m_skybox.get_index_offset()));
        m_skybox.unbind();
        ::glDepthFunc(GL_LESS);
        ::glEnable(GL_CULL_FACE);
    }
}