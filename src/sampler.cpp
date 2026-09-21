#include "sampler.hpp"
#include "opengl.hpp"
namespace game
{
    Sampler::Sampler() 
        :m_handle{0u, [](auto sampler){::glDeleteSamplers(1, &sampler);}}
    {
        ::glCreateSamplers(1, &m_handle);
        
    }
    Sampler::Sampler(SamplerUsage usage)
    :m_handle{0u, [](auto sampler){::glDeleteSamplers(1, &sampler);}}
    {
        ::glCreateSamplers(1, &m_handle);
        float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        switch (usage) 
        {
            using enum SamplerUsage;
            case SHADOWMAP:
                ::glSamplerParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                ::glSamplerParameterfv(m_handle, GL_TEXTURE_BORDER_COLOR, borderColor);
                break;
            case DEPTHTEXTURE:
                ::glSamplerParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                ::glSamplerParameterfv(m_handle, GL_TEXTURE_BORDER_COLOR, borderColor);
                break;
            case COLORTEXTURE:
                ::glSamplerParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                ::glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
        }
    }
    ::GLuint Sampler::get_native_handle() const { return m_handle; }

} // namespace game
