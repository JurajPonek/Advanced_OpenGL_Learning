#pragma once

#include "auto_release.hpp"
#include "camera.hpp"
#include "entity.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "window.hpp"
#include <gl/gl.h>
#include <memory>
#include <vector>
namespace game
{
    class FrameBufferScene : public Scene
    {
        public:
          FrameBufferScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer);
          virtual void on_render() override;
          virtual void on_imgui_render() override;
          virtual void on_attach() override;
          virtual void on_detach() override;
        private:
          std::vector<Entity> m_entities;
          Camera* m_camera;
          std::unique_ptr<Texture> m_texture;
          std::unique_ptr<Texture> m_texture_spec;
          std::unique_ptr<Sampler> m_sampler;
          std::unique_ptr<Mesh> m_cube;
          std::unique_ptr<Mesh> m_plane;
          std::unique_ptr<Mesh> m_sphere;
          std::unique_ptr<Material> m_material;
          std::unique_ptr<Material> m_post_process_material;
          Renderer* m_renderer;
          FrameBuffer m_fbo;
    };
}