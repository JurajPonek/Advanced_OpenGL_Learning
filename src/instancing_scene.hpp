#pragma once

#include "buffer.hpp"
#include "camera.hpp"
#include "cubemap.hpp"
#include "entity.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "matrix4.hpp"
#include "mesh.hpp"
#include "mesh_loader.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "window.hpp"
#include <array>
#include <cstddef>
#include <gl/gl.h>
#include <memory>
#include <vector>

namespace game
{
    class InstancingScene : public Scene
    {
    static constexpr size_t INSTANCES_COUNT = 100000;
      public:
        InstancingScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer, MeshLoader* mesh_loader);
        virtual void on_render() override;
        virtual void on_attach() override;

      private:
        std::vector<Entity> m_entities;
        Camera* m_camera;
        std::unique_ptr<Texture> m_planet_texture;
        std::unique_ptr<Texture> m_rock_texture;
        std::unique_ptr<Sampler> m_planet_sampler;
        std::unique_ptr<Sampler> m_rock_sampler;
        std::unique_ptr<Material> m_rock_material;
        std::unique_ptr<Material> m_planet_material;
        std::unique_ptr<Mesh> m_planet;
        std::unique_ptr<Mesh> m_rock;
        Renderer* m_renderer;
        MeshLoader* m_mesh_loader;
        std::unique_ptr<Entity> m_rock_entity;
        std::vector<Matrix4> m_transforms;
        Buffer m_transforms_buffer;
    };
}


