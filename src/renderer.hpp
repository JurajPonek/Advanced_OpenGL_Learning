#pragma once

#include "auto_release.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "cubemap.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "matrix4.hpp"
#include "mesh.hpp"
#include "mesh_loader.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "src/matrix4.hpp"
#include <gl/gl.h>
#include <span>
#include <tuple>

namespace game
{
    class Renderer
    {
      public:
        Renderer(MeshLoader& mesh_loader, ResourceLoader& resource_loader);
        void set_camera(const Camera* camera);
        void draw_mesh(const Mesh* mesh, const Material* material, const Matrix4& transform,
                       std::span<const std::tuple<const Texture*, const Sampler*>> textures) const;
        void draw_instanced(const Mesh* mesh, const Material* material, std::span<const std::tuple<const Texture*, const Sampler*>> textures, size_t count) const;
        void draw_post_process_texture(const Material* material, const Sampler* sampler, FrameBuffer* fbo);
        void draw_skybox(CubeMap* cubemap, Sampler* sampler) const;

      private:
        Material setup_skybox_material(ResourceLoader& resource_loader) const;

      private:
        Buffer m_camera_buffer;
        AutoRelease<::GLuint> m_post_process_vao;
        Mesh m_skybox;
        Material m_skybox_material;
    };
} // namespace game