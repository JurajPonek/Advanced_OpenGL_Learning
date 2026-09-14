#pragma once

#include "buffer.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "entity.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include "window.hpp"
#include <memory>
#include <vector>

namespace game
{

    class LightningScene : public Scene
    {
        struct DirectionalLight
        {
            Vector3 direction;
            Color color;
        };
        struct PointLight
        {
            Vector3 position;
            Color color;
            float const_attenuation;
            float linear_attenuation;
            float quad_attenuation;
        };

      public:
        LightningScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer);
        virtual void on_render() override;
        virtual void on_update(float dt) override;
        virtual void on_imgui_render() override;
        virtual void on_attach() override;
        virtual void on_detach() override;

      private:
        std::vector<Entity> m_entities;
        Color m_ambient;
        DirectionalLight m_directional;
        std::vector<PointLight> m_points;
        Camera* m_camera;
        Buffer m_light_buffer;
        std::unique_ptr<Texture> m_texture;
        std::unique_ptr<Texture> m_texture_spec;
        std::unique_ptr<Sampler> m_sampler;
        std::unique_ptr<Material> m_material;
        std::unique_ptr<Mesh> m_mesh;
        Renderer* m_renderer;
    };
} // namespace game