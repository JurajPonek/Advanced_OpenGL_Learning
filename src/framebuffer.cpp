#include "framebuffer.hpp"
#include "error.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include "vendor/opengl/glext.h"
#include <cstddef>
#include <gl/gl.h>
#include <memory>

namespace
{
    bool is_depth_format(game::TextureFormat format)
    {
        return (format == game::TextureFormat::Depth24Stencil8 || format == game::TextureFormat::Depth32F) ? true
                                                                                                           : false;
    }
} // namespace

namespace game
{
    FrameBuffer::FrameBuffer(const FramebufferSpecification& spec)
        : m_handle{0u, [](const auto buffer) { ::glDeleteFramebuffers(1, &buffer); }}, m_specification{spec}
    {
        ::glCreateFramebuffers(1, &m_handle);
        std::vector<GLenum> draw_buffers;
        for (const auto& attachment_format : m_specification.attachments)
        {
            TextureSpecification texture_spec;
            texture_spec.width = m_specification.width;
            texture_spec.height = m_specification.height;
            texture_spec.type = m_specification.type;
            texture_spec.samples = m_specification.samples;
            texture_spec.format = attachment_format;
            texture_spec.max_lights = m_specification.max_lights;
            if (is_depth_format(attachment_format))
            {
                m_depth_attachment.emplace(texture_spec);
                GLenum attachment_point = (attachment_format == TextureFormat::Depth24Stencil8)
                                              ? GL_DEPTH_STENCIL_ATTACHMENT
                                              : GL_DEPTH_ATTACHMENT;
                ::glNamedFramebufferTexture(m_handle, attachment_point, m_depth_attachment->get_native_handle(), 0);
            }
            else
            {
                size_t index = m_color_attachments.size();
                m_color_attachments.push_back({texture_spec});
                GLenum attachment_point = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index);

                ::glNamedFramebufferTexture(m_handle, attachment_point, m_color_attachments.back().get_native_handle(),
                                            0);
                draw_buffers.push_back(attachment_point);
            }
        }
        if (draw_buffers.empty())
        {
            ::glNamedFramebufferDrawBuffer(m_handle, GL_NONE);
            ::glNamedFramebufferReadBuffer(m_handle, GL_NONE);
        }
        else
        {
            ::glNamedFramebufferDrawBuffers(m_handle, m_color_attachments.size(), draw_buffers.data());
        }
        ensure(::glCheckNamedFramebufferStatus(m_handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
               "Failed to complete framebuffer");
    }


    ::GLuint FrameBuffer::get_native_handle() const { return m_handle; }
    void FrameBuffer::bind() const
    {
        ::glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
        ::glViewport(0, 0, m_specification.width, m_specification.height);
    }
    void FrameBuffer::unbind() const { ::glBindFramebuffer(GL_FRAMEBUFFER, 0); }
    const Texture& FrameBuffer::get_color_attachment(size_t index) const { return m_color_attachments[index]; }
    const Texture& FrameBuffer::get_depth_attachment() const
    {
        ensure(m_depth_attachment, "Framebuffer doest have depth attachment");
        return *m_depth_attachment;
    }
    std::uint32_t FrameBuffer::get_width() const { return m_specification.width; }
    std::uint32_t FrameBuffer::get_height() const { return m_specification.height; }
} // namespace game
