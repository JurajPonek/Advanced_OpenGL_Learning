#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <span>
#include "src/error.hpp"
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

            static constexpr Matrix4 orthographic(float left, float right, float bottom, float top, float znear, float zfar)
            {
                Matrix4 res{};
                res.m_data[0] = 2.0f / (right - left);
                res.m_data[5] = 2.0f / (top - bottom);
                res.m_data[10] = -2.0f / (zfar - znear);
                res.m_data[12] = -((right + left) / (right - left));
                res.m_data[13] = -((top + bottom) / (top - bottom));
                res.m_data[14] = -((zfar + znear) / (zfar - znear));
                return res;
            }

            friend constexpr Matrix4&
            operator*=(Matrix4& mat1, const Matrix4& mat2);
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
                Matrix4 tmp{mat.m_data};
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

            inline static constexpr Matrix4 translate(const Matrix4& mat, Vector3 translation)
            {
                Matrix4 tmp{mat.m_data};
                tmp.m_data[12] = translation.x;
                tmp.m_data[13] = translation.y;
                tmp.m_data[14] = translation.z;
                return tmp;

            }
            inline static constexpr Matrix4 scale(const Matrix4& mat, Vector3 scale)
            {
                Matrix4 tmp{mat.m_data};
                tmp.m_data[0] = scale.x;
                tmp.m_data[5] = scale.y;
                tmp.m_data[10] = scale.z;
                return tmp;
            }

            inline static constexpr Matrix4 inverse(const Matrix4& mat)
            {
                float s0 = (mat.m_data[0] * mat.m_data[5]) - (mat.m_data[1] * mat.m_data[4]);
                float s1 = (mat.m_data[0] * mat.m_data[6]) - (mat.m_data[2] * mat.m_data[4]);
                float s2 = (mat.m_data[0] * mat.m_data[7]) - (mat.m_data[3] * mat.m_data[4]);
                float s3 = (mat.m_data[1] * mat.m_data[6]) - (mat.m_data[2] * mat.m_data[5]);
                float s4 = (mat.m_data[1] * mat.m_data[7]) - (mat.m_data[3] * mat.m_data[5]);
                float s5 = (mat.m_data[2] * mat.m_data[7]) - (mat.m_data[3] * mat.m_data[6]);

                float c5 = (mat.m_data[10] * mat.m_data[15]) - (mat.m_data[11] * mat.m_data[14]);
                float c4 = (mat.m_data[9] * mat.m_data[15]) - (mat.m_data[11] * mat.m_data[13]);
                float c3 = (mat.m_data[9] * mat.m_data[14]) - (mat.m_data[10] * mat.m_data[13]);
                float c2 = (mat.m_data[8] * mat.m_data[15]) - (mat.m_data[11] * mat.m_data[12]);
                float c1 = (mat.m_data[8] * mat.m_data[14]) - (mat.m_data[10] * mat.m_data[12]);
                float c0 = (mat.m_data[8] * mat.m_data[13]) - (mat.m_data[9] * mat.m_data[12]);

                float det = (s0 * c5) - (s1 * c4) + (s2 * c3) + (s3 * c2) - (s4 * c1) + (s5 * c0);

                ensure((det > 0.000001f || det < -0.000001f), "Cannot compute inverse matrix, determinant is 0");

                float inv_det = 1.0f / det;
                Matrix4 res{};

                res.m_data[0] = (mat.m_data[5] * c5 - mat.m_data[6] * c4 + mat.m_data[7] * c3) * inv_det;
                res.m_data[1] = (-mat.m_data[1] * c5 + mat.m_data[2] * c4 - mat.m_data[3] * c3) * inv_det;
                res.m_data[2] = (mat.m_data[13] * s5 - mat.m_data[14] * s4 + mat.m_data[15] * s3) * inv_det;
                res.m_data[3] = (-mat.m_data[9] * s5 + mat.m_data[10] * s4 - mat.m_data[11] * s3) * inv_det;

                res.m_data[4] = (-mat.m_data[4] * c5 + mat.m_data[6] * c2 - mat.m_data[7] * c1) * inv_det;
                res.m_data[5] = (mat.m_data[0] * c5 - mat.m_data[2] * c2 + mat.m_data[3] * c1) * inv_det;
                res.m_data[6] = (-mat.m_data[12] * s5 + mat.m_data[14] * s2 - mat.m_data[15] * s1) * inv_det;
                res.m_data[7] = (mat.m_data[8] * s5 - mat.m_data[10] * s2 + mat.m_data[11] * s1) * inv_det;

                res.m_data[8] = (mat.m_data[4] * c4 - mat.m_data[5] * c2 + mat.m_data[7] * c0) * inv_det;
                res.m_data[9] = (-mat.m_data[0] * c4 + mat.m_data[1] * c2 - mat.m_data[3] * c0) * inv_det;
                res.m_data[10] = (mat.m_data[12] * s4 - mat.m_data[13] * s2 + mat.m_data[15] * s0) * inv_det;
                res.m_data[11] = (-mat.m_data[8] * s4 + mat.m_data[9] * s2 - mat.m_data[11] * s0) * inv_det;

                res.m_data[12] = (-mat.m_data[4] * c3 + mat.m_data[5] * c1 - mat.m_data[6] * c0) * inv_det;
                res.m_data[13] = (mat.m_data[0] * c3 - mat.m_data[1] * c1 + mat.m_data[2] * c0) * inv_det;
                res.m_data[14] = (-mat.m_data[12] * s3 + mat.m_data[13] * s1 - mat.m_data[14] * s0) * inv_det;
                res.m_data[15] = (mat.m_data[8] * s3 - mat.m_data[9] * s1 + mat.m_data[10] * s0) * inv_det;

                return res;
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