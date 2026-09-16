#pragma once
#include "cubemap.hpp"
#include "opengl.hpp"
#include "auto_release.hpp"
#include "sampler.hpp"
#include "shader.hpp"
#include "src/matrix4.hpp"
#include "string_map.hpp"
#include "texture.hpp"
#include <cstdint>
#include <gl/gl.h>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace game
{
    class Material
    {
        public:
            Material(const Shader& vertex_shader, const Shader& fragment_shader);
            GLuint get_native_handle() const;
            void use() const;
            void set_uniform(std::string_view name, const Matrix4& data) const;
            void set_uniform(std::string_view name, int obj) const;
            void bind_texture(std::uint32_t index, const Texture* texture, const Sampler* sampler) const;
            void bind_textures(std::span<const std::tuple<const Texture*, const Sampler*>> tex_samps) const;
            void bind_cubemap(const CubeMap* texture, const Sampler* sampler) const;
        private:
            AutoRelease<GLuint> m_handle;
            StringMap<::GLuint> m_uniforms;


    };
}