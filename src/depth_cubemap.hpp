#pragma  once

#include "auto_release.hpp"
#include "opengl.hpp"

namespace game
{
    class DepthCubeMap
    {
      public:
        DepthCubeMap(std::uint32_t width, std::uint32_t height, int max_lights);
        ::GLuint get_native_handle() const;


      private:
        AutoRelease<::GLuint> m_handle;
    };
} // namespace game