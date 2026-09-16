#pragma once

#include "auto_release.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include <cstdint>
#include <gl/gl.h>
namespace game
{
    class FrameBuffer
    {
        public:
            FrameBuffer(std::uint32_t width, std::uint32_t height);
            ::GLuint get_native_handle() const;
            void bind() const;
            void unbind() const;
            const Texture& get_color_attachment() const;
            const Texture& get_depth_attachment() const;
            std::uint32_t get_width() const;
            std::uint32_t get_height() const;
        private:
            AutoRelease<::GLuint> m_handle;
            Texture m_color_attachment;
            Texture m_depth_attachment;
            std::uint32_t m_width;
            std::uint32_t m_height;
    };

}