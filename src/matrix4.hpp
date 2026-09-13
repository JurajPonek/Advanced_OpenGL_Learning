#pragma once

#include <array>
#include <cstddef>
#include <span>
#include "src/vector3.hpp"
#include "vector3.hpp"

namespace game
{
    class Matrix4
    {
        public:
            constexpr Matrix4()
                : m_data({1.0f,0.0f,0.0f,0.0f,
                            0.0f,1.0f,0.0f,0.0f,
                            0.0f,0.0f,1.0f,0.0f,
                            0.0f,0.0f,0.0f,1.0f})
            {

            }

            constexpr Matrix4(const std::array<float, 16>& data)
                : m_data(data)
            {

            }

            constexpr Matrix4(const Vector3& translation)
                : m_data({1.0f,0.0f,0.0f,0.0f,
                            0.0f,1.0f,0.0f,0.0f,
                            0.0f,0.0f,1.0f,0.0f,
                            translation.x,translation.y,translation.z,1.0f})
            {

            }
            constexpr Matrix4(const Vector3& translation, const Vector3& scale)
                : m_data({scale.x, 0.0f, 0.0f, 0.0f, 0.0f, scale.y, 0.0f, 0.0f, 0.0f, 0.0f, scale.z, 0.0f, translation.x,
                          translation.y, translation.z, 1.0f})
            {
            }

            constexpr std::span<const float, 16> data() const
            {
                return m_data;
            }


            static constexpr Matrix4 look_at(const Vector3& position, const Vector3& target, const Vector3& up)
            {
                const auto direction = Vector3::normalize(target - position);
                const auto up_normalized = Vector3::normalize(up);
                const auto camera_right = Vector3::normalize(Vector3::cross(direction, up_normalized)); 
                const auto camera_up = Vector3::normalize(Vector3::cross(camera_right, direction));
                auto matrix = Matrix4{};
                matrix.m_data = {camera_right.x, camera_up.x, -direction.x, 0.0f,
                                camera_right.y, camera_up.y, -direction.y, 0.0f,
                                camera_right.z, camera_up.z, -direction.z, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f};



                return matrix * Matrix4(-position);
            }
            friend constexpr Matrix4& operator*=(Matrix4& mat1, const Matrix4& mat2);
            friend constexpr Matrix4 operator*(const Matrix4& mat1, const Matrix4& mat2);
            
            inline static constexpr Matrix4 perspective(float fov_radians, float width, float height, float near_p, float far_p)
            {
                const float aspect = width / height;
                const float tan_half_fov = std::tan(fov_radians / 2.0f); 

                Matrix4 matrix{};
                matrix.m_data.fill(0.0f);
                matrix.m_data[0] = 1.0f / (aspect * tan_half_fov);
                matrix.m_data[5] = 1.0f / tan_half_fov;
                matrix.m_data[10] = -(far_p + near_p) / (far_p - near_p);
                matrix.m_data[11] = -1.0f;
                matrix.m_data[14] = -(2.0f * far_p * near_p) / (far_p - near_p);
                matrix.m_data[15] = 0.0f;

                return matrix;
            }   
        private:
            std::array<float, 16> m_data;

    };

    constexpr Matrix4& operator*=(Matrix4& mat1, const Matrix4& mat2)
    {
        auto res = Matrix4{};
        for (size_t i {0}; i < 4; i++)
        {
            for (size_t j {0}; j < 4; j++)
            {
                res.m_data[i + j * 4] = 0.0f;
                for (size_t k {0}; k < 4; k++)
                {
                    res.m_data[i + j * 4] += mat1.m_data[i + k * 4] * mat2.m_data[k + j * 4];
                }
            }
        }
        mat1 = res;
        return mat1;
    }

    constexpr Matrix4 operator*(const Matrix4& mat1, const Matrix4& mat2)
    {
        auto tmp = Matrix4{mat1};
        return tmp *= mat2;
    }


}