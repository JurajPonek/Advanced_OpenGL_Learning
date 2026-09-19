#include "framebuffer.hpp"
#include "error.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include "vendor/opengl/glext.h"
#include <cstddef>
#include <gl/gl.h>

namespace game
{
    FrameBuffer::FrameBuffer(const FramebufferSpecification& spec)
        : m_handle{0u, [](const auto buffer){::glDeleteFramebuffers(1, &buffer);}},
        m_specification{spec}
    {
        
        ::glCreateFramebuffers(1, &m_handle);
        for (auto usage : m_specification.attachments)
        {
            if (usage == TextureUsage::DEPTHATTACHMENT)
            {
                m_depth_attachment.emplace(usage, m_specification.width, m_specification.height, m_specification.samples);
                ::glNamedFramebufferTexture(m_handle, GL_DEPTH_ATTACHMENT,m_depth_attachment->get_native_handle(), 0);
            }
            else 
            {
                size_t index = m_color_attachments.size();
                m_color_attachments.emplace_back(usage, m_specification.width, m_specification.height, m_specification.samples);
                ::glNamedFramebufferTexture(m_handle, GL_COLOR_ATTACHMENT0 + index,
                                                m_color_attachments.back().get_native_handle(), 0);
            }
        }
        if (m_color_attachments.empty())
        {
            ::glNamedFramebufferDrawBuffer(m_handle, GL_NONE);
            ::glNamedFramebufferReadBuffer(m_handle, GL_NONE);
        }
        else
        {
            ensure(m_color_attachments.size() < 4, "Exceeded color attachment limit");
            GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                                 GL_COLOR_ATTACHMENT3};
            ::glNamedFramebufferDrawBuffers(m_handle, m_color_attachments.size(), buffers);
        }
        ensure(::glCheckNamedFramebufferStatus(m_handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Failed to complete framebuffer");
    }
    ::GLuint FrameBuffer::get_native_handle() const
    {
        return m_handle;
    }
    void FrameBuffer::bind() const
    {
        ::glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
        ::glViewport(0, 0, m_specification.width, m_specification.height);
    }
    void FrameBuffer::unbind() const
    {
        ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    const Texture& FrameBuffer::get_color_attachment(size_t index) const
    {
        return m_color_attachments[index];
    }
    const Texture& FrameBuffer::get_depth_attachment() const
    {
        ensure(m_depth_attachment, "Framebuffer doest have depth attachment");
        return *m_depth_attachment;
    }
    std::uint32_t FrameBuffer::get_width() const
    {
        return m_specification.width;
    }
    std::uint32_t FrameBuffer::get_height() const
    {
        return m_specification.height;
    }



}
