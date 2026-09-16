#include "framebuffer.hpp"
#include "error.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include "vendor/opengl/glext.h"
#include <gl/gl.h>

namespace game
{
    FrameBuffer::FrameBuffer(std::uint32_t width, std::uint32_t height)
        : m_handle{0u, [](const auto buffer){::glDeleteFramebuffers(1, &buffer);}},
        m_color_attachment{TextureUsage::COLORATTACHMENT, width, height},
        m_depth_attachment{TextureUsage::DEPTHATTACHMENT, width, height},
        m_width{width},
        m_height{height}
    {
        ::glCreateFramebuffers(1, &m_handle);
        ::glNamedFramebufferTexture(m_handle, GL_COLOR_ATTACHMENT0, m_color_attachment.get_native_handle(), 0);
        ::glNamedFramebufferTexture(m_handle, GL_DEPTH_ATTACHMENT,m_depth_attachment.get_native_handle(), 0);
        ensure(::glCheckNamedFramebufferStatus(m_handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Failed to complete framebuffer");
    }
    ::GLuint FrameBuffer::get_native_handle() const
    {
        return m_handle;
    }
    void FrameBuffer::bind() const
    {
        ::glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
        ::glViewport(0, 0, m_width, m_height);
    }
    void FrameBuffer::unbind() const
    {
        ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ::glViewport(0, 0, 1920, 1080);
    }
    const Texture& FrameBuffer::get_color_attachment() const
    {
        return m_color_attachment;
    }
    const Texture& FrameBuffer::get_depth_attachment() const
    {
        return m_depth_attachment;
    }
    std::uint32_t FrameBuffer::get_width() const
    {
        return m_width;
    }
    std::uint32_t FrameBuffer::get_height() const
    {
        return m_height;
    }



}
