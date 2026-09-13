#pragma once

#include "buffer.hpp"
#include "opengl.hpp"
#include "auto_release.hpp"
#include "mesh_loader.hpp"
#include <cstdint>
#include <gl/gl.h>

namespace game
{
    class Mesh
    {
        public:
            Mesh(const MeshData& data);
            void bind() const;
            void unbind() const;
            std::uint32_t get_index_count() const;
            std::uintptr_t get_index_offset() const;
          private:
            AutoRelease<::GLuint> m_vao;
            Buffer m_vbo;
            std::uint32_t m_index_count;
            std::uintptr_t m_index_offset;

    };
}

