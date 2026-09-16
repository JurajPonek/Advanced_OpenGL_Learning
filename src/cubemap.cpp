#include "cubemap.hpp"
#include "error.hpp"
#include "opengl.hpp"
#include "resource_loader.hpp"
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

    ::GLuint CubeMap::get_native_handle() const
    {
        return m_handle;
    }
}