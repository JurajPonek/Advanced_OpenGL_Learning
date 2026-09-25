#pragma once

#include "auto_release.hpp"
#include "opengl.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace game
{
    enum class TextureType
    {
        TEXTURE2D,
        DEPTHCUBEMAP,

    };

    enum class TextureFormat
    {
        RGBA8,
        R32I,
        Depth24Stencil8,
        SRGBA,
        Depth32F
    };

    struct TextureSpecification
    {
        uint32_t width = 0;
        uint32_t height = 0;
        TextureType type;
        TextureFormat format;
        bool generate_mipmaps = false;
        bool default_normal_map_texture = false;
        bool default_height_map_texture = false;
        std::uint32_t max_lights = 0;
        std::uint32_t samples = 1;
    };

    class Texture
    {
      public:
        Texture(std::span<const std::byte> data);
        Texture(std::span<const std::byte> data, TextureFormat format);
        Texture(std::span<const std::byte> data, std::uint32_t width, std::uint32_t height);
        Texture(const TextureSpecification& spec);
      


        ::GLuint get_native_handle() const;

      private:
        AutoRelease<::GLuint> m_handle;
    };

} // namespace game