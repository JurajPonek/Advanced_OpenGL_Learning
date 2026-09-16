#pragma once

#include <span>
#include "matrix4.hpp"
#include "vector3.hpp"

namespace game
{   
    class Camera
    {
        public:
            Camera(const Vector3& position, const Vector3& look_at, const Vector3& up,
                    float fov, float width, float height, float near_plane, float far_plane);

            std::span<const float> get_view() const;
            std::span<const float> get_projection() const;
            void translate(const Vector3& translation);
            Vector3 get_direction() const;
            Vector3 get_right() const;
            Vector3 get_position() const;
            void adjust_yaw(float adjust);
            void adjust_pitch(float adjust);
            void rotate(float angle, Vector3 vector);
        private:
            Matrix4 m_view;
            Matrix4 m_projection;
            Vector3 m_position;
            Vector3 m_direction;
            Vector3 m_up;
            float m_pitch;
            float m_yaw;
                

    };

}




