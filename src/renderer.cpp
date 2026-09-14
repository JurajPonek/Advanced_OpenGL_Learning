#include "renderer.hpp"
#include "buffer_writer.hpp"
#include "color.hpp"
#include "material.hpp"
#include "opengl.hpp"
#include "matrix4.hpp"
#include "Dscene.hpp"
#include "vector3.hpp"
#include <gl/gl.h>
#include <ranges>
#include "camera.hpp"
#include "vendor/opengl/glext.h"

namespace
{
    struct PointLightBufer
    {
        alignas(16) game::Vector3 point;
        alignas(16) game::Color color;
        alignas(16) game::Vector3 attenuation;
    };
    struct LightBuffer
    {
        alignas(16) game::Color ambient;
        alignas(16) game::Vector3 direction;
        alignas(16) game::Color direction_color;
        int num_points;
        
    };

}


namespace game
{
    Renderer::Renderer() : 
    m_camera_buffer {sizeof(Matrix4) * 2 + sizeof(Vector3)}, m_light_buffer(10240u)
    {

    }
    void Renderer::render(const Camera& camera, const DScene& scene) const
    {
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {
            BufferWriter writer{m_camera_buffer};
            writer.write(camera.get_view());
            writer.write(camera.get_projection());
            writer.write(camera.get_position());
        }

        {
            LightBuffer light_buffer{scene.ambient, scene.directional.direction, scene.directional.color, static_cast<int>(scene.points.size())};
            BufferWriter writer{m_light_buffer};
            writer.write(light_buffer);
            for (const auto& point : scene.points)
            {
                auto point_light_buffer = PointLightBufer{point.position, point.color, {point.const_attenuation, point.linear_attenuation, point.quad_attenuation}};
                writer.write(point_light_buffer);
            }
        }
        ::glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_camera_buffer.get_native_handle());
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_light_buffer.get_native_handle());
        for(const auto* entity : scene.m_entities)
        {
            const auto* material = entity->get_material();         
            const auto* mesh = entity->get_mesh();
            material->use();
            material->set_uniform("model", entity->get_model_matrix());
            material->bind_textures(entity->get_textures()); 
            mesh->bind();
            ::glDrawElements(GL_TRIANGLES, mesh->get_index_count(), GL_UNSIGNED_INT, reinterpret_cast<void*>(mesh->get_index_offset()));
            mesh->unbind();
        }
    }


}