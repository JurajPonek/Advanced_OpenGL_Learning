#pragma once

#include "camera.hpp"
#include "material.hpp"
#include "renderer.hpp"
#include "resource_loader.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "src/framebuffer.hpp"
#include "texture.hpp"
#include "window.hpp"
#include <memory>
#include <vector>

namespace game
{
    class ComputeTestingScene : public Scene
    {
      public:
        ComputeTestingScene(const ResourceLoader& loader, Window* window, Camera* camera, Renderer* renderer);
        virtual void on_render() override;

      private:
        std::unique_ptr<Material> m_compute_material;
        std::unique_ptr<FrameBuffer> m_fbo;
        std::unique_ptr<Material> m_default_material;
        std::unique_ptr<Texture> m_test_texture;
        Camera* m_camera;
        Window* m_window;
        Renderer* m_renderer;
        std::unique_ptr<Sampler> m_sampler;
        Buffer m_test_buffer;
        std::vector<float> m_test_data;
    };
} // namespace game