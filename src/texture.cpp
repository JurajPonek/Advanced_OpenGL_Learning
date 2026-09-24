#include "texture.hpp"
#include "error.hpp"
#include "opengl.hpp"
#include "vendor/opengl/glext.h"
#include <cstddef>
#include <cstdint>
#include <gl/gl.h>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace
{

    GLenum to_gl_internal_format(const game::TextureFormat& format)
    {
        switch (format)
        {
            using enum game::TextureFormat;
        case RGBA8:
            return GL_RGBA8;
        case R32I:
            return GL_R32I;
        case Depth24Stencil8:
            return GL_DEPTH24_STENCIL8;
        case Depth32F:
            return GL_DEPTH_COMPONENT32F;
        case SRGBA:
            return GL_SRGB8_ALPHA8;
        default:
        return GL_NONE;
        }
    }
    bool is_depth_format(game::TextureFormat format)
    {
        return format == game::TextureFormat::Depth24Stencil8 || format == game::TextureFormat::Depth32F;
    }
} // namespace

namespace game
{
    Texture::Texture(std::span<const std::byte> data)
        : m_handle{0u, [](auto texture) { ::glDeleteTextures(1u, &texture); }}
    {
        int w{};
        int h{};
        int num_channels{};
        std::unique_ptr<::stbi_uc, decltype(&::stbi_image_free)> raw_data{
            ::stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data.data()), static_cast<int>(data.size()), &w,
                                    &h, &num_channels, 4),
            ::stbi_image_free};
        ensure(raw_data, "Failed to parse  texture data");
        const auto width = static_cast<std::uint32_t>(w);
        const auto height = static_cast<std::uint32_t>(h);
        ::glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
        ::glTextureStorage2D(m_handle, 1, GL_RGBA8, width, height);
        ::glTextureSubImage2D(m_handle, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, raw_data.get());
    }
    Texture::Texture(std::span<const std::byte> data, TextureFormat format)
        : m_handle{0u, [](auto texture) { ::glDeleteTextures(1u, &texture); }}
    {
        int w{};
        int h{};
        int num_channels{};
        ::GLenum gl_format = format == TextureFormat::SRGBA ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        std::unique_ptr<::stbi_uc, decltype(&::stbi_image_free)> raw_data{
            ::stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data.data()), static_cast<int>(data.size()), &w,
                                    &h, &num_channels, 4),
            ::stbi_image_free};
        ensure(raw_data, "Failed to parse  texture data");
        const auto width = static_cast<std::uint32_t>(w);
        const auto height = static_cast<std::uint32_t>(h);
        ::glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
        ::glTextureStorage2D(m_handle, 1, gl_format, width, height);
        ::glTextureSubImage2D(m_handle, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, raw_data.get());
    }
    Texture::Texture(std::span<const std::byte> data, std::uint32_t width, std::uint32_t height)
        : m_handle{0u, [](auto texture) { ::glDeleteTextures(1u, &texture); }}
    {
        int w{};
        int h{};
        auto num_channels = int{3};
        std::unique_ptr<::stbi_uc, decltype(&::stbi_image_free)> raw_data{
            ::stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data.data()), static_cast<int>(data.size()), &w,
                                    &h, &num_channels, 4),
            ::stbi_image_free};
        ensure(raw_data, "Failed to parse  texture data");
        ensure(static_cast<std::uint32_t>(w) == width, "Width has changed");
        ensure(static_cast<std::uint32_t>(h) == height, "Height has changed");
        ::glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
        ::glTextureStorage2D(m_handle, 1, GL_RGBA8, width, height);
        ::glTextureSubImage2D(m_handle, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, raw_data.get());
    }

    Texture::Texture(const TextureSpecification& spec) : m_handle{0u, [](auto tex) { ::glDeleteTextures(1u, &tex); }}
    {

        if (spec.default_normal_map_texture)
        {
            unsigned char flatBluePixel[4] = {128, 128, 255, 255};
            ::glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
            ::glTextureStorage2D(m_handle, 1, GL_RGBA8, 1, 1);
            ::glTextureSubImage2D(m_handle, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, flatBluePixel);
            ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
            return;
        }
        if (spec.type == TextureType::TEXTURE2D)
        {
            if (spec.samples == 1)
            {
                ::glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
                ::glTextureStorage2D(m_handle, 1, to_gl_internal_format(spec.format), spec.width, spec.height);
                GLenum filter = is_depth_format(spec.format) ? GL_NEAREST : GL_LINEAR;
                ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, filter);
                ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, filter);
                ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            else 
            {
                ::glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_handle);
                ::glTextureStorage2DMultisample(m_handle, spec.samples, to_gl_internal_format(spec.format), spec.width, spec.height, GL_TRUE);
            }
        }
        else if (spec.type == TextureType::DEPTHCUBEMAP)
        {
            ::glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_handle);
            ::glTextureStorage3D(m_handle, 1, to_gl_internal_format(spec.format), spec.width, spec.height,
                                 spec.max_lights * 6);
            ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
    }
    ::GLuint Texture::get_native_handle() const { return m_handle; }
} // namespace game
