#include "instancing_scene.hpp"
#include "buffer_writer.hpp"
#include "matrix4.hpp"
#include "opengl.hpp"
#include "vendor/opengl/glext.h"
#include <cmath>
#include <cstddef>
#include <memory>
#include <numbers>
#include <random>





namespace game 
{
    InstancingScene::InstancingScene(ResourceLoader& resource_loader, Window* window, Camera* camera,
                                     Renderer* renderer, MeshLoader* mesh_loader)
        : m_entities{}, m_camera{camera}, m_renderer{renderer}, m_mesh_loader{mesh_loader}, m_transforms{},
          m_transforms_buffer{sizeof(Matrix4) * INSTANCES_COUNT}
    {
        m_planet_texture = std::make_unique<Texture>(resource_loader.load_binary("mars.png"));
        m_rock_texture = std::make_unique<Texture>(resource_loader.load_binary("rock.png"));
        m_planet_sampler = std::make_unique<Sampler>();
        m_rock_sampler = std::make_unique<Sampler>();
        const Texture* textures1[]{m_planet_texture.get() };
        const Texture* textures2[]{m_rock_texture.get()};
        const Sampler* samplers1[]{m_planet_sampler.get() };
        const Sampler* samplers2[]{m_rock_sampler.get()};
        const auto tex_samp1 = std::views::zip(textures1, samplers1) | std::ranges::to<std::vector>();
        const auto tex_samp2 = std::views::zip(textures2, samplers2) | std::ranges::to<std::vector>();
        const auto rock_vertex_shader =
            Shader(resource_loader.load_string("shaders/instancing_vert.glsl"), game::ShaderType::VERTEX);
        const auto rock_fragment_shader =
            Shader(resource_loader.load_string("shaders/instancing_frag.glsl"), game::ShaderType::FRAGMENT);
        const auto planet_vertex_shader =
            Shader(resource_loader.load_string("shaders/default_vert.glsl"), game::ShaderType::VERTEX);
        const auto planet_fragment_shader =
            Shader(resource_loader.load_string("shaders/default_frag.glsl"), game::ShaderType::FRAGMENT);
        m_rock_material = std::make_unique<Material>(rock_vertex_shader, rock_fragment_shader);
        m_planet_material = std::make_unique<Material>(planet_vertex_shader, planet_fragment_shader);
        m_planet = std::make_unique<Mesh>(mesh_loader->load("planet.obj"));
        m_rock = std::make_unique<Mesh>(mesh_loader->load("rock.obj"));
        m_entities.emplace_back(m_planet.get(), m_planet_material.get(), Vector3{0.0f, -3.0f, 0.0f}, Vector3{4.0f},
                                tex_samp1);
        m_rock_entity = std::make_unique<Entity>(m_rock.get(), m_rock_material.get(), Vector3{1.0f, 1.0f, 1.0f}, Vector3{1.0f}, tex_samp2);

        m_transforms.reserve(INSTANCES_COUNT);
        std::random_device rd;
        std::mt19937 gen(rd());
        float radius = 150.0f;
        float offset = 25.0f;
        std::uniform_real_distribution<float> displaceDist(-offset, offset); 
        std::uniform_real_distribution<float> scaleDist(0.05f, 0.25f);       
        std::uniform_real_distribution<float> rotAngleDist(0.0f, 360.0f);

        for (size_t i{0}; i < INSTANCES_COUNT; ++i)
        {
            Matrix4 model{};

            float angle = static_cast<float>(i) / static_cast<float>(INSTANCES_COUNT) * 360.0f;
            float rad = angle * (std::numbers::pi_v<float> / 180.0f);
            float x = std::sin(rad) * radius + displaceDist(gen);
            float y = displaceDist(gen) * 0.4f;
            float z = std::cos(rad) * radius + displaceDist(gen);

            model = Matrix4::translate(model, {x, y, z});

            float scale = scaleDist(gen);
            model = Matrix4::scale(model, {scale});

            float rotAngle = rotAngleDist(gen);
            model = Matrix4::rotate(model, rotAngle, {0.4f, 0.6f, 0.8f});

            m_transforms.push_back(model);
        }
    }
    void InstancingScene::on_render()
    {
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_renderer->set_camera(m_camera);
        for (const auto& entity : m_entities)
        {
            m_renderer->draw_mesh(entity.get_mesh(), entity.get_material(), entity.get_model_matrix(),
                                  entity.get_textures());
        }

        BufferWriter writer{m_transforms_buffer};
        for (size_t i{0}; i < INSTANCES_COUNT; ++i)
        {
            writer.write(m_transforms[i]);
        }
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_transforms_buffer.get_native_handle());

        m_renderer->draw_instanced(m_rock_entity->get_mesh(), m_rock_entity->get_material(),
                                       m_rock_entity->get_textures(), INSTANCES_COUNT);
    }
    void InstancingScene::on_attach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
}