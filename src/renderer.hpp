#pragma once

#include "buffer.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "matrix4.hpp"
#include "mesh.hpp"
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

          private:
            Buffer m_camera_buffer;
    };
}