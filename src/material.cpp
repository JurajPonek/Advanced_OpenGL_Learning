
#include "material.hpp"
#include "shader.hpp"

#include "error.hpp"
#include "opengl.hpp"
#include "src/log.hpp"
#include "vendor/opengl/glext.h"
#include <cstdint>
#include <gl/gl.h>
#include <ranges>
#include <string>
namespace game
{

    Material::Material(const Shader& vertex_shader, const Shader& fragment_shader) : m_handle({}), m_uniforms{}
    {
        ensure(vertex_shader.get_type() == ShaderType::VERTEX,  "Shader is not vertex shader");
        ensure(fragment_shader.get_type() == ShaderType::FRAGMENT, "Shader is not fragment shader");

        m_handle = game::AutoRelease<::GLuint>
        {
            ::glCreateProgram(), ::glDeleteProgram
        };
        ensure(m_handle, "failed to create program");
        ::glAttachShader(m_handle, vertex_shader.get_native_handle());
        ::glAttachShader(m_handle, fragment_shader.get_native_handle());
        ::glLinkProgram(m_handle);
        GLint res{};
        ::glGetProgramiv(m_handle, GL_LINK_STATUS, &res);
        if (res != GL_TRUE)
        {
            char log[512];
            ::glGetProgramInfoLog(m_handle, sizeof(log), nullptr, log);

            ensure(res, "Failed to link program\n{}", log);
        }
        ::GLint uniform_count{};
        ::glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &uniform_count);
        if (uniform_count != 0)
        {
            ::GLint max_name_lenght{};
            ::glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_name_lenght);
            log::debug("Max name lenght {}", max_name_lenght);
            ::GLsizei lenght{};
            ::GLsizei count{};
            ::GLenum type{};
            for (auto i{0}; i < uniform_count; i++)
            {
                std::string name(max_name_lenght, '\0');
                ::glGetActiveUniform(m_handle, i, max_name_lenght, &lenght, &count, &type, name.data());
                name.resize(lenght);
                const auto location = ::glGetUniformLocation(m_handle, name.c_str());
                m_uniforms[name] = location;
                log::debug("Found unifrom {}", name);
            }

        }
        log::info("new material ({} uniforms)", uniform_count);


    }
    Material::Material(const Shader& vertex_shader, const Shader& geometry_shader, const Shader& fragment_shader)
    {
        ensure(vertex_shader.get_type() == ShaderType::VERTEX, "Shader is not vertex shader");
        ensure(geometry_shader.get_type() == ShaderType::GEOMETRY, "Shader is not geometry shader");
        ensure(fragment_shader.get_type() == ShaderType::FRAGMENT, "Shader is not fragment shader");

        m_handle = game::AutoRelease<::GLuint>{::glCreateProgram(), ::glDeleteProgram};
        ensure(m_handle, "failed to create program");
        ::glAttachShader(m_handle, vertex_shader.get_native_handle());
        ::glAttachShader(m_handle, geometry_shader.get_native_handle());
        ::glAttachShader(m_handle, fragment_shader.get_native_handle());
        ::glLinkProgram(m_handle);
        GLint res{};
        ::glGetProgramiv(m_handle, GL_LINK_STATUS, &res);
        if (res != GL_TRUE)
        {
            char log[512];
            ::glGetProgramInfoLog(m_handle, sizeof(log), nullptr, log);

            ensure(res, "Failed to link program\n{}", log);
        }
        ::GLint uniform_count{};
        ::glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &uniform_count);
        if (uniform_count != 0)
        {
            ::GLint max_name_lenght{};
            ::glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_name_lenght);
            log::debug("Max name lenght {}", max_name_lenght);
            ::GLsizei lenght{};
            ::GLsizei count{};
            ::GLenum type{};
            for (auto i{0}; i < uniform_count; i++)
            {
                std::string name(max_name_lenght, '\0');
                ::glGetActiveUniform(m_handle, i, max_name_lenght, &lenght, &count, &type, name.data());
                name.resize(lenght);
                const auto location = ::glGetUniformLocation(m_handle, name.c_str());
                m_uniforms[name] = location;
                log::debug("Found unifrom {}", name);
            }
        }
        log::info("new material ({} uniforms)", uniform_count);
    }
    GLuint Material::get_native_handle() const
    {
        return m_handle;
    }
    void Material::use() const
    {
        ::glUseProgram(m_handle);
    }
    void Material::set_uniform(std::string_view name, const Matrix4& data) const
    {
        const auto uniform = m_uniforms.find(name);
        ensure(uniform != std::ranges::cend(m_uniforms), "missing uniform {}", name);
        ::glUniformMatrix4fv(uniform->second, 1, GL_FALSE, data.data().data());
    }

    void Material::set_uniform(std::string_view name, int obj) const
    {
        const auto uniform = m_uniforms.find(name);
        ensure(uniform != std::ranges::cend(m_uniforms), "missing uniform {}", name);
        ::glUniform1i(uniform->second, obj);
    }
    void Material::bind_texture(std::uint32_t index, const Texture* texture, const Sampler* sampler) const
    {
        ::glBindTextureUnit(index, texture->get_native_handle());
        ::glBindSampler(index, sampler->get_native_handle());
        const auto uniform_name = std::format("tex{}", index);
        set_uniform(uniform_name, index);
    }
    void Material::bind_textures(std::span<const std::tuple<const Texture*, const Sampler*>> tex_samps) const
    {
        for (const auto& [index, tex_samp] :tex_samps | std::views::enumerate)
        {
            const auto& [texture, sampler] = tex_samp;
            bind_texture(static_cast<std::uint32_t>(index), texture , sampler);
        }
    }
    void Material::bind_cubemap(const CubeMap* texture, const Sampler* sampler) const
    {
        ::glBindTextureUnit(0, texture->get_native_handle());
        ::glBindSampler(0, sampler->get_native_handle());
        const auto uniform_name = std::format("tex{}", 0);
        set_uniform(uniform_name, 0);
    }
}