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
    Texture::Texture(TextureUsage usage, std::uint32_t width, std::uint32_t height, size_t samples)
        : m_handle{0u, [](auto tex) { ::glDeleteTextures(1u, &tex); }}
    {

        ::GLenum gl_usage = samples == 1 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;
        ::glCreateTextures(gl_usage, 1, &m_handle);

        
        switch (usage)
        {
            using enum TextureUsage;
        case COLORATTACHMENT:
            if (samples > 1)
            {
                ::glTextureStorage2DMultisample(m_handle, samples, GL_RGBA8, width, height, GL_TRUE);
                break;
            }
            else
            {
                ::glTextureStorage2D(m_handle, 1, GL_RGBA8, width, height);
                break;
            }

        case DEPTHATTACHMENT:
            if (samples > 1)
            {
                ::glTextureStorage2DMultisample(m_handle, samples, GL_DEPTH_COMPONENT24, width, height, GL_TRUE);
                break;
            }
            else
            {
                ::glTextureStorage2D(m_handle, 1, GL_DEPTH_COMPONENT24, width, height);
                break;
            }
            
        }

        if (samples == 1 && usage == TextureUsage::COLORATTACHMENT)
        {
            ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else if (samples == 1 && usage == TextureUsage::DEPTHATTACHMENT)
        {
            ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
            ::glTextureParameterfv(m_handle, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
    }
    ::GLuint Texture::get_native_handle() const { return m_handle; }
} // namespace game
