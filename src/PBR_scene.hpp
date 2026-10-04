#pragma once

#include "auto_release.hpp"
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
#include "src/matrix4.hpp"
#include "texture.hpp"
#include "vector3.hpp"
#include "window.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <gl/gl.h>
#include <memory>
#include <vector>


namespace game
{

    class PBRScene : public Scene
    {
        static constexpr int SSAO_KERNEL_SIZE = 64;
        struct PointLight
        {
            Vector3 position;
            Color color;
            float radius;
            float intensity;
            int shadow_map_index;
        };
        struct DirectionalLight
        {
            Vector3 direction;
            Color color;
        };

      public:
        PBRScene(ResourceLoader& resource_loader, Window* window, Camera* camera, Renderer* renderer,
                               MeshLoader* mesh_loader);
        virtual void on_render() override;
        virtual void on_imgui_render() override;
        virtual void on_attach() override;
        virtual void on_detach() override;

      private:
        void setup_lights() const;
        void setup_shadows(const Matrix4& lightSpaceMatrix) const;
        void setup_point_shadows() const;
        std::array<Matrix4, 6> calculate_shadow_transformations(const PointLight& point) const;
        void setup_other_uniforms() const;
        void execute_g_pass() const;
        void execute_shadow_pass(const Matrix4& light_space_matrix) const;
        void execute_lighting_pass(const Matrix4& light_space_matrix) const;
        void execute_post_process_pass() const;
        void execute_ssao_pass() const;
        Matrix4 calculate_light_space_matrix() const;
        void setup_textures_from_g_buffer() const;
        void debug_draw(const Texture& attachment) const;

      private:
        std::vector<Entity> m_entities;
        Camera* m_camera;
        std::unique_ptr<Texture> m_ssao_noise_texture;
        std::unique_ptr<Sampler> m_sampler;
        std::unique_ptr<Sampler> m_shadow_map_sampler;
        std::unique_ptr<Mesh> m_sphere;
        std::unique_ptr<Material> m_material;
        std::unique_ptr<Material> m_post_process_material;
        std::unique_ptr<Material> m_shadow_map_material;
        std::unique_ptr<Material> m_point_shadows_material;
        std::unique_ptr<Material> m_g_buffer_material;
        std::unique_ptr<Material> m_ssao_material;
        std::unique_ptr<Material> m_ssao_blur_material;
        std::unique_ptr<Material> m_debug_view_material;
        std::unique_ptr<Material> m_pbr_test_material;
        std::unique_ptr<Texture> m_sphere_albedo;
        std::unique_ptr<Texture> m_sphere_normal;
        std::unique_ptr<Texture> m_sphere_metallic;
        std::unique_ptr<Texture> m_sphere_roughness;
        std::unique_ptr<Texture> m_sphere_height;
        std::unique_ptr<Texture> m_sphere_ao;
        std::unique_ptr<Texture> m_default_normal_map_texture;
        std::unique_ptr<Texture> m_default_height_map_texture;
        Renderer* m_renderer;
        std::unique_ptr<FrameBuffer> m_post_process_fbo;
        std::unique_ptr<FrameBuffer> m_shadow_map;
        std::unique_ptr<FrameBuffer> m_omnidirectional_shadow_map;
        std::unique_ptr<FrameBuffer> m_g_buffer;
        std::unique_ptr<FrameBuffer> m_ssao_fbo;
        std::unique_ptr<FrameBuffer> m_ssao_blur_fbo;
        std::unique_ptr<FrameBuffer> m_debug_fbo;
        MeshLoader* m_mesh_loader;
        std::vector<PointLight> m_points;
        Buffer m_light_buffer;
        DirectionalLight m_directional;
        Color m_ambient;
        Matrix4 m_shadow_proj;
        std::vector<Vector3> m_ssao_kernel;
    };
} // namespace game