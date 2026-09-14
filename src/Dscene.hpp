#pragma once

#include "entity.hpp"
#include "color.hpp"
#include "scene.hpp"
#include "vector3.hpp"
#include <vector>



namespace game
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
    class DScene 
    {
        public:
        const std::vector<const Entity*> m_entities;
        Color ambient;
        DirectionalLight directional;
        std::vector<PointLight> points;
    };

} // namespace game
