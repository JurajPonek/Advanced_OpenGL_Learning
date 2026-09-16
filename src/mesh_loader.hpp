#pragma once

#include "resource_loader.hpp"
#include "src/string_map.hpp"
#include "src/vertex_data.hpp"
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
namespace game
{
    struct MeshData
    {
        std::span<const VertexData> vertices;
        std::span<const std::uint32_t> indices;
    };
    class MeshLoader
    {
        public:
            MeshLoader(ResourceLoader& resource_loader);
            MeshData cube();
            MeshData plane();
            MeshData sphere();
            MeshData load(std::string_view mesh_file, std::string_view mesh_name);
            MeshData load(std::string_view mesh_file);
          private:
            struct LoadedMeshData
            {
                std::vector<VertexData> vertices;
                std::vector<std::uint32_t> indices;
            };
            StringMap<LoadedMeshData> m_loaded_meshes;
            ResourceLoader& m_resource_loader;
    };


}