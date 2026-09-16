#pragma once

#include "auto_release.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "matrix4.hpp"
#include "mesh.hpp"
#include "sampler.hpp"
#include <gl/gl.h>
#include <tuple>
#include <vector>

namespace game
{
    class Renderer
    {
        public:
            Renderer();
            void set_camera(const Camera* camera);
            void draw_mesh(const Mesh* mesh, const Material* material, const Matrix4& transform,
                           std::span<const std::tuple<const Texture*, const Sampler*>> textures) const;
            void draw_post_process_texture(const Material* material, const Sampler* sampler, const FrameBuffer& fbo);

          private:
            Buffer m_camera_buffer;
            AutoRelease<::GLuint> m_post_process_vao;
    };
}