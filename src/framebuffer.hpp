#pragma once

#include "auto_release.hpp"
#include "depth_cubemap.hpp"
#include "opengl.hpp"
#include "texture.hpp"
#include <cstddef>
#include <cstdint>
#include <gl/gl.h>
#include <memory>
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
        const DepthCubeMap& get_depth_cubemap_attachment() const;
        std::uint32_t get_width() const;
        std::uint32_t get_height() const;

      private:
        AutoRelease<::GLuint> m_handle;
        FramebufferSpecification m_specification;
        std::vector<std::unique_ptr<Texture>> m_color_attachments;
        std::unique_ptr<Texture> m_depth_attachment;
        std::unique_ptr<DepthCubeMap> m_depth_cube_map;
    };

} // namespace game