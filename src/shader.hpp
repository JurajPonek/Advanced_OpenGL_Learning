#pragma once
#include <format>
#include <string_view>
#include <utility>
#include "auto_release.hpp"
#include "exception.hpp"
#include "opengl.hpp"

namespace game
{
    enum class ShaderType
    {
        VERTEX,
        FRAGMENT,
        GEOMETRY
    };

    class Shader
    {
        public:
            Shader(std::string_view source, ShaderType type);
            ShaderType get_type() const;
            ::GLuint get_native_handle() const;

        private:
            AutoRelease<::GLuint> m_handle;
            ShaderType m_type;

    };


}

template<>
    struct std::formatter<game::ShaderType>
    {
        constexpr auto parse(std::format_parse_context& ctx)
        {
            return std::cbegin(ctx);
        }
        auto format(const game::ShaderType& shader_type, std::format_context& ctx) const
        {
            switch (shader_type) 
            {
                using enum game::ShaderType;
                case VERTEX:
                    return std::format_to(ctx.out(), "{}\n", "VERTEX");
                case FRAGMENT:
                    return std::format_to(ctx.out(), "{}\n", "FRAGMENT");
                case GEOMETRY:
                    return std::format_to(ctx.out(), "{}\n", "GEOMETRY");
                }
            throw game::Exception("Unknown shader type", std::to_underlying(shader_type));
        }  
    };