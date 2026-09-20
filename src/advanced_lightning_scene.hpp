#pragma once

#include "buffer.hpp"
#include "camera.hpp"
#include "cubemap.hpp"
#include "entity.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "mesh_loader.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include "window.hpp"
#include <gl/gl.h>
#include <memory>
#include <vector>


namespace game
{

    class AdvancedLightningScene : public Scene
    {
        struct PointLight
        {
            Vector3 position;
            Color color;
            int shininess;
            float radius;
            float intensity;
        };
        struct DirectionalLight
        {
            Vector3 direction;
            Color color;
        };

      public:
        AdvancedLightningScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer,
                               MeshLoader* mesh_loader);
        virtual void on_render() override;
        virtual void on_imgui_render() override;
        virtual void on_attach() override;
        virtual void on_detach() override;

      private:
            void setup_lights() const;
      private:
        std::vector<Entity> m_entities;
        Camera* m_camera;
        std::unique_ptr<Texture> m_default_texture;
        std::unique_ptr<Texture> m_plane_texture;
        std::unique_ptr<Sampler> m_sampler;
        std::unique_ptr<Mesh> m_cube;
        std::unique_ptr<Mesh> m_plane;
        std::unique_ptr<Mesh> m_sphere;
        std::unique_ptr<Material> m_material;
        std::unique_ptr<Material> m_post_process_material;
        Renderer* m_renderer;
        std::unique_ptr<FrameBuffer> m_msaa_fbo;
        std::unique_ptr<FrameBuffer> m_post_process_fbo;
        MeshLoader* m_mesh_loader;
        std::vector<PointLight> m_points;
        Buffer m_light_buffer;
        DirectionalLight m_directional;
        Color m_ambient;
    };
} // namespace game