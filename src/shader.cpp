
#include "shader.hpp"
#include "error.hpp"
#include "opengl.hpp"
#include "vendor/opengl/glext.h"
#include <gl/gl.h>
#include <stdexcept>



namespace
{
    GLenum to_native_shader_type(game::ShaderType type)
    {
        switch (type)
        {
            using enum game::ShaderType;
        case VERTEX:
            return GL_VERTEX_SHADER;
        case FRAGMENT:
            return GL_FRAGMENT_SHADER;
        case GEOMETRY:
            return GL_GEOMETRY_SHADER;
        }
        throw std::runtime_error("Unknown shader type");
    }
} // namespace

namespace game
{
    Shader::Shader(std::string_view source, ShaderType type) : m_handle({}), m_type(type)
    {
        m_handle = game::AutoRelease<::GLuint>{::glCreateShader(to_native_shader_type(type)), ::glDeleteShader};
        const char* string[] = {source.data()};
        const ::GLint lenghts[] = {static_cast<::GLint>(source.length())};
        ::glShaderSource(m_handle, 1, string, lenghts);
        ::glCompileShader(m_handle);

        ::GLint res{};
        ::glGetShaderiv(m_handle, GL_COMPILE_STATUS, &res);
        if (res != GL_TRUE)
        {
            char log[512];
            ::glGetShaderInfoLog(m_handle, sizeof(log), nullptr, log);

            ensure(res, "Failed to compile {} shader\n{}", m_type, log);
        }

        
    }
    ShaderType Shader::get_type() const { return m_type; }
    ::GLuint Shader::get_native_handle() const { return m_handle; }


} // namespace game
