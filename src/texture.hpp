#pragma once

#include "auto_release.hpp"
#include "opengl.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace game
{
    enum class TextureUsage
    {
        COLORATTACHMENT,
        DEPTHATTACHMENT
    };
    class Texture
    {
      
      public:
        Texture(std::span<const std::byte> data, std::uint32_t width, std::uint32_t height);
        Texture(TextureUsage usage, std::uint32_t width, std::uint32_t height);
        
        ::GLuint get_native_handle() const;

      private:
        AutoRelease<::GLuint> m_handle;
    };

} // namespace game