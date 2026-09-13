#include "entity.hpp"
#include "matrix4.hpp"
#include "sampler.hpp"
#include "src/vector3.hpp"
#include "texture.hpp"
#include <tuple>
#include <vector>

namespace game
{
    Entity::Entity(const Mesh* mesh, const Material* material, const Vector3& position,const Vector3& scale, const std::vector<std::tuple<const Texture*, const Sampler*>>& textures)
    :   m_model{position, scale}, m_mesh(mesh), m_material(material), m_textures{textures} 
    {

    }

    const Matrix4& Entity::get_model_matrix() const
    
    {
        return m_model;
    }

    const Mesh* Entity::get_mesh() const
    {
        return m_mesh;
    }
    const Material* Entity::get_material() const
    {
        return m_material;
    }
    std::span<const std::tuple<const Texture*, const Sampler*>> Entity::get_textures() const
    {
        return m_textures;
    }


}