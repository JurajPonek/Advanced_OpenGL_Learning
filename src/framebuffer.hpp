#pragma once

#include "auto_release.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include <cstddef>
#include <cstdint>
#include <gl/gl.h>
#include <optional>
#include <vector>

namespace game
{
    struct FramebufferSpecification
    {
        FramebufferSpecification() = default;
        std::uint32_t width, height;
        size_t samples = 1;
        std::vector<TextureUsage> attachments;
    };
    class FrameBuffer
    {

      public:
        FrameBuffer(const FramebufferSpecification& spec);
        ::GLuint get_native_handle() const;
        void bind() const;
        void unbind() const;
        const Texture& get_color_attachment(size_t index = 0) const;
        const Texture& get_depth_attachment() const;
        std::uint32_t get_width() const;
        std::uint32_t get_height() const;

      private:
        AutoRelease<::GLuint> m_handle;
        FramebufferSpecification m_specification;
        std::vector<Texture> m_color_attachments;
        std::optional<Texture> m_depth_attachment;
    };

} // namespace game