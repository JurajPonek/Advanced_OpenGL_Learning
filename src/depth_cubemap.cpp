#include "depth_cubemap.hpp"
#include "opengl.hpp"
#include "vendor/opengl/glext.h"
#include <cstddef>
#include <cstdint>
#include <gl/gl.h>

namespace game
{
    DepthCubeMap::DepthCubeMap(std::uint32_t width, std::uint32_t height, int max_lights)
    :m_handle{0u, [](auto tex){::glDeleteTextures(1, &tex);}}
    {
        ::glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_handle);
        ::glTextureStorage3D(m_handle, 1, GL_DEPTH_COMPONENT24, width, height, max_lights * 6);
        ::glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        ::glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ::glTextureParameteri(m_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    ::GLuint DepthCubeMap::get_native_handle() const
    {
        return m_handle;
    }
}


