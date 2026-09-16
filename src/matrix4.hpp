#pragma once

#include <array>
#include <cmath>
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
            friend constexpr Vector3 operator*(const Matrix4& mat, const Vector3& vec);

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

            inline static constexpr Matrix4 rotate(const Matrix4& mat, float angle, Vector3 vector)
            {
                auto c = std::cos(angle);
                auto s = std::sin(angle);
                auto t = 1 - c;
                vector = Vector3::normalize(vector);
                Matrix4 tmp{};
                tmp.m_data[0]= t * vector.x * vector.x + c;
                tmp.m_data[4] = t * vector.x * vector.y + vector.z * s;
                tmp.m_data[8] = t * vector.x * vector.z - vector.y * s;
                tmp.m_data[1] = t * vector.x * vector.y - vector.z * s;
                tmp.m_data[5] = t * vector.y * vector.y + c;
                tmp.m_data[9] = t * vector.z * vector.y + vector.x * s;
                tmp.m_data[2] = t * vector.x * vector.z +  vector.y * s;
                tmp.m_data[6] = t * vector.y * vector.z - vector.x * s;
                tmp.m_data[10] = t * vector.z * vector.z + c;
                return mat * tmp;

            }

          private:
            std::array<float, 16> m_data;

    };

    constexpr Vector3 operator*(const Matrix4& mat, const Vector3& vec) 
    {
        Vector3 res{};
        res.x = mat.m_data[0] * vec.x + mat.m_data[1] * vec.y + mat.m_data[2] * vec.z;
        res.y = mat.m_data[4] * vec.x + mat.m_data[5] * vec.y + mat.m_data[6] * vec.z;
        res.z = mat.m_data[8] * vec.x + mat.m_data[9] * vec.y + mat.m_data[10] * vec.z;
        return res;    
    }
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