
#include "mesh.hpp"
#include "auto_release.hpp"
#include "buffer_writer.hpp"
#include "mesh_loader.hpp"
#include "opengl.hpp"
#include "mesh_loader.hpp"
#include "vendor/opengl/glext.h"
#include "vertex_data.hpp"
#include <cstddef>
#include <cstdint>
#include <gl/gl.h>
#include <iterator>



namespace game
{
    Mesh::Mesh(const MeshData& data)
        : m_vao({0u, [](auto vao) { ::glDeleteVertexArrays(1, &vao); }}),
          m_vbo{static_cast<std::uint32_t>(data.vertices.size_bytes() + data.indices.size_bytes())}, m_index_count(static_cast<std::uint32_t>(data.indices.size())),
          m_index_offset(data.vertices.size_bytes())
    {
        {
            BufferWriter writer{m_vbo};
            writer.write(data.vertices);
            writer.write(data.indices);
        }

        ::glCreateVertexArrays(1, &m_vao);
        ::glVertexArrayVertexBuffer(m_vao, 0, m_vbo.get_native_handle(), 0, sizeof(VertexData));
        ::glVertexArrayElementBuffer(m_vao, m_vbo.get_native_handle());

        ::glEnableVertexArrayAttrib(m_vao, 0);
        ::glEnableVertexArrayAttrib(m_vao, 1);
        ::glEnableVertexArrayAttrib(m_vao, 2);
        ::glEnableVertexArrayAttrib(m_vao, 3);

        ::glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexData, position));
        ::glVertexArrayAttribFormat(m_vao,  1, 3, GL_FLOAT, GL_FALSE, offsetof(VertexData, normal));
        ::glVertexArrayAttribFormat(m_vao,  2, 3, GL_FLOAT, GL_FALSE, offsetof(VertexData, tangent));
        ::glVertexArrayAttribFormat(m_vao, 3, 2, GL_FLOAT, GL_FALSE, offsetof(VertexData, uv));

        ::glVertexArrayAttribBinding(m_vao, 0, 0);
        ::glVertexArrayAttribBinding(m_vao, 1, 0);
        ::glVertexArrayAttribBinding(m_vao, 2, 0);
        ::glVertexArrayAttribBinding(m_vao, 3, 0);
    }
    void Mesh::bind() const { ::glBindVertexArray(m_vao); }
    void Mesh::unbind() const { ::glBindVertexArray(0); }
    std::uint32_t Mesh::get_index_count() const
    {
        return m_index_count;
    }
    std::uintptr_t Mesh::get_index_offset() const
    {
        return m_index_offset;
    }

} // namespace game
