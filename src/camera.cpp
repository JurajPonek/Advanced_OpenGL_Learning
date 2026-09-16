#include "camera.hpp"
#include "matrix4.hpp"
#include "src/matrix4.hpp"
#include "vector3.hpp"
#include <cmath>
#include <span>

namespace
{
    game::Vector3 create_direction(float pitch, float yaw)
    {
        return game::Vector3::normalize(
            game::Vector3{std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch)});
    }
} // namespace

namespace game
{
    Camera::Camera(const Vector3& position, const Vector3& look_at, const Vector3& up, float fov, float width,
                   float height, float near_plane, float far_plane)
        : m_view{Matrix4::look_at(position, look_at, up)},
          m_projection{
              Matrix4::perspective(fov, width, height, near_plane, far_plane),
          },
          m_position{position}, m_direction{look_at}, m_up(up), m_pitch{}, m_yaw{}

    {
        m_direction = create_direction(m_pitch, m_yaw);
        m_view = Matrix4::look_at(m_position, m_position + m_direction, m_up);
    }


    std::span<const float> Camera::get_view() const { return m_view.data(); }
    std::span<const float> Camera::get_projection() const { return m_projection.data(); }

    void Camera::translate(const Vector3& translation)
    {
        m_position += translation;
        m_view = Matrix4::look_at(m_position, m_direction + m_position, m_up);
    }
    Vector3 Camera::get_direction() const { return m_direction; }
    Vector3 Camera::get_right() const { return Vector3::normalize(Vector3::cross(m_direction, m_up)); }

    void Camera::adjust_yaw(float adjust)
    {
        m_yaw += adjust;
        m_direction = create_direction(m_pitch, m_yaw);
        m_view = Matrix4::look_at(m_position, m_position + m_direction, m_up);
    }
    void Camera::adjust_pitch(float adjust)
    {
        m_pitch += adjust;
        m_direction = create_direction(m_pitch, m_yaw);
        m_view = Matrix4::look_at(m_position, m_position + m_direction, m_up);
    }
    Vector3 Camera::get_position() const { return m_position; }

    void Camera::rotate(float angle, Vector3 vector)
    {
        Matrix4 mat{};
        auto tmp = Matrix4::rotate(mat, angle, vector);
        m_direction = Vector3::normalize(tmp * m_direction);
        m_up = Vector3::normalize(tmp * m_up);
        m_view = Matrix4::look_at(m_position, m_position + m_direction, m_up);
        m_pitch = std::asin(m_direction.y);
        m_yaw = std::atan2(m_direction.z, m_direction.x);

    }

} // namespace game
