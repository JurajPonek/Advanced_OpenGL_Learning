#include "cubemap.hpp"
#include "auto_release.hpp"
#include "error.hpp"
#include "material.hpp"
#include "opengl.hpp"
#include "resource_loader.hpp"
#include "shader.hpp"
#include "vendor/opengl/glext.h"
#include "stb_image.h"
#include <array>
#include <cstddef>
#include <gl/gl.h>
#include <memory>
#include <string>
#include <vector>
#include <ranges>
namespace game
{
    
    CubeMap::CubeMap(const std::vector<std::string>& faces, const ResourceLoader& loader)
        :m_handle{0u, [](auto tex){::glDeleteTextures(1, &tex);}}
    {
        ::glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_handle);
        std::array<std::vector<std::byte>, 6> all_data{};
        for (const auto& [index, face] : faces | std::views::enumerate)
        {
            all_data[index] = loader.load_binary(face); 
        }
        int width{};
        int height{};
        int channels{};
        const auto info = ::stbi_info_from_memory(reinterpret_cast<const stbi_uc*>(all_data[0].data()),
                                                     static_cast<int>(all_data[0].size()), &width, &height, &channels);
        ensure(info, "Failed to read cubemap image");
        ensure(width == height, "Cubemap faces must be square");
        ::glTextureStorage2D(m_handle, 1, GL_RGBA8,width, height);
        for (size_t face{0}; face < 6; ++face)
        {
            int w{};
            int h{};
            int num_channels{};
            std::unique_ptr<::stbi_uc, decltype(&::stbi_image_free)> raw_data{
                ::stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(all_data[face].data()),
                                        static_cast<int>(all_data[face].size()), &w, &h, &num_channels, 4),
                ::stbi_image_free};
            ensure(raw_data, "Failed to parse cubemap data");
            ensure(w == width && h == height, "All cubemap faces must have the same size!");

            ::glTextureSubImage3D(m_handle, 0, 0, 0, static_cast<GLint>(face), w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, raw_data.get());
        }
        ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    CubeMap::CubeMap(const std::string& hrd_map_name, const ResourceLoader& loader)
     :m_handle{0u, [](auto tex){::glDeleteTextures(1, &tex);}}
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, num_of_components;
        const auto bytes = loader.load_binary(hrd_map_name);
        float* data = stbi_loadf_from_memory(reinterpret_cast<const stbi_uc*>(bytes.data()),
                                             static_cast<int>(bytes.size()), &width, &height, &num_of_components, 4);
        ensure(data, "Failed to load hdr environment map");
        AutoRelease<GLuint, 0u> hdr_texture{0u, [](auto tex) { ::glDeleteTextures(1, &tex); }};
        ::glCreateTextures(GL_TEXTURE_2D, 1, &hdr_texture);
        ::glTextureStorage2D(hdr_texture, 1, GL_RGBA16F, width, height);
        ::glTextureSubImage2D(hdr_texture, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);
        ::glTextureParameteri(hdr_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ::glTextureParameteri(hdr_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ::glTextureParameteri(hdr_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        ::glTextureParameteri(hdr_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        stbi_image_free(data);

        constexpr int cube_size = 2048;
        ::glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_handle);
        ::glTextureStorage2D(m_handle, 1, GL_RGBA16F, cube_size, cube_size);
        ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        const auto hdr_to_cubemap_compute = Shader{loader.load_string("shaders/hdr_to_cubemap_compute.glsl"), ShaderType::COMPUTE};
        const auto mat = Material{hdr_to_cubemap_compute};
        mat.use();
        ::glBindTextureUnit(0, hdr_texture);
        ::glBindImageTexture(1, m_handle, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        ::glDispatchCompute((cube_size + 15) / 16, (cube_size + 15) / 16, 6);
        ::glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    }

    ::GLuint CubeMap::get_native_handle() const
    {
        return m_handle;
    }
}