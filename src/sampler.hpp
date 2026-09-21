#pragma once

#include "opengl.hpp"
#include "auto_release.hpp"

enum class SamplerUsage
{
  SHADOWMAP,
  DEPTHTEXTURE,
  COLORTEXTURE
};

namespace game
{
    class Sampler
    {
      public:
        Sampler();
        Sampler(SamplerUsage usage);
        ::GLuint get_native_handle() const; 
      private:
        AutoRelease<::GLuint> m_handle;
    };
} // namespace Sampler