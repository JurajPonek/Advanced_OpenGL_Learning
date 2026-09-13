#pragma once

#include "material.hpp"
#include "matrix4.hpp"
#include "mesh.hpp"
#include "sampler.hpp"
#include "src/vector3.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include <span>
#include <tuple>
#include <vector>


namespace game
{
    class Entity
    {
      public:
        Entity(const Mesh* mesh, const Material* material, const Vector3& position, const Vector3& scale,
               const std::vector<std::tuple<const Texture*, const Sampler*>>& textures);

        const Matrix4& get_model_matrix() const;
        const Mesh* get_mesh() const;
        const Material* get_material() const;
        std::span<const std::tuple<const Texture*, const Sampler*>> get_textures() const;

      private:
        Matrix4 m_model;
        const Mesh* m_mesh;
        const Material* m_material;
        std::vector<std::tuple<const Texture*, const Sampler*>> m_textures;
    };

} // namespace game