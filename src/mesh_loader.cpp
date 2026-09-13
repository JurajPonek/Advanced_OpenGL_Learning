#include "mesh_loader.hpp"
#include "assimp/mesh.h"
#include "assimp/vector3.h"
#include "error.hpp"
#include "resource_loader.hpp"
#include "src/vector3.hpp"
#include "src/vertex_data.hpp"
#include "vector3.hpp"
#include <cstdint>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "log.hpp"
namespace
{
    template <typename... Args> std::vector<game::VertexData> vertices(Args&&... args)
    {
        return std::views::zip_transform([]<typename... A>(A&&... a)
                                         { return game::VertexData{std::forward<A>(a)...}; },
                                         std::forward<Args>(args)...) |
               std::ranges::to<std::vector>();
    }
} // namespace

namespace game
{
    MeshLoader::MeshLoader(ResourceLoader& resource_loader)
        :m_resource_loader(resource_loader)
    {

    }

    MeshData MeshLoader::load(std::string_view model_file, std::string_view model_name)
    {
        const auto model_file_data = m_resource_loader.load_binary(model_file);
        ensure(!model_file_data.empty(), "No model data!");
        ::Assimp::Importer importer{};
        const auto* scene = importer.ReadFileFromMemory(model_file_data.data(), model_file_data.size(), ::aiProcess_Triangulate | ::aiProcess_FlipUVs | ::aiProcess_CalcTangentSpace);
        ensure((scene != nullptr) && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "failed to load model {} {}", model_file, model_name);
        const auto loaded_meshes = std::span<::aiMesh*>{scene->mMeshes, scene->mNumMeshes};
        log::debug("Found meshes {}", loaded_meshes.size());
        for (const auto* mesh : loaded_meshes)
        {
            const auto loaded = m_loaded_meshes.find(mesh->mName.C_Str());
            if (loaded != std::ranges::cend(m_loaded_meshes))
            {
                continue;
            }
            const auto to_vector3 = [](const ::aiVector3D& v){return Vector3{v.x, v.y, v.z};};
            const auto positions = std::span<::aiVector3D>{mesh->mVertices, mesh->mVertices + mesh->mNumVertices} |
                                   std::views::transform(to_vector3);

            const auto normals = std::span<::aiVector3D>{mesh->mNormals, mesh->mNormals + mesh->mNumVertices} | std::views::transform(to_vector3);
            std::vector<UV> uvs{};
            ensure(mesh->HasTextureCoords(0), "Mesh doesnt have tex coords");
            for (auto i{0u}; i < mesh->mNumVertices; ++i)
            {
                uvs.push_back({mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y});
            }

            auto indices = std::vector<std::uint32_t>{};
            for (auto i{0u}; i < mesh->mNumFaces; ++i)
            {
                const auto& face = mesh->mFaces[i];
                for (auto j{0u}; j < face.mNumIndices; ++j)
                {
                    indices.push_back(face.mIndices[j]);
                }
            }
            
                m_loaded_meshes.emplace(mesh->mName.C_Str(), LoadedMeshData{vertices(positions, normals, uvs), std::move(indices)});
        }

        const auto loaded = m_loaded_meshes.find(model_name);
        ensure(loaded != std::ranges::cend(m_loaded_meshes), "Failed to load {} from {}", model_name, model_file);
        return {loaded->second.vertices, loaded->second.indices};
    }
    MeshData MeshLoader::load(std::string_view model_file)
    {
        // 1. Cache
        const auto loaded = m_loaded_meshes.find(model_file);
        if (loaded != std::ranges::cend(m_loaded_meshes))
        {
            return {loaded->second.vertices, loaded->second.indices};
        }

        // 2. Načítanie súboru
        const auto model_file_data = m_resource_loader.load_binary(model_file);
        ensure(!model_file_data.empty(), "No model data for file {}", model_file);

        ::Assimp::Importer importer{};
        const auto* scene = importer.ReadFileFromMemory(model_file_data.data(), model_file_data.size(),
                                                        ::aiProcess_Triangulate | ::aiProcess_CalcTangentSpace |
                                                            ::aiProcess_GenSmoothNormals |
                                                            ::aiProcess_PreTransformVertices | ::aiProcess_FlipUVs,
                                                        "fbx");
        //::aiProcess_FlipUVs

        ensure((scene != nullptr) && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mNumMeshes > 0,
               "Failed to load model {}", model_file);

        std::vector<VertexData> all_vertices;
        std::vector<std::uint32_t> all_indices;

        // 3. Prejdeme VŠETKY sub-meshe v modeli a spojíme ich
        for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
        {
            const auto* mesh = scene->mMeshes[m];

            // Offset pre indexy tohto sub-meshu
            const auto vertex_offset = static_cast<std::uint32_t>(all_vertices.size());

            const auto to_vector3 = [](const ::aiVector3D& v) { return Vector3{v.x, v.y, v.z}; };
            const auto positions = std::span<::aiVector3D>{mesh->mVertices, mesh->mVertices + mesh->mNumVertices} |
                                   std::views::transform(to_vector3);

            const auto normals = std::span<::aiVector3D>{mesh->mNormals, mesh->mNormals + mesh->mNumVertices} |
                                 std::views::transform(to_vector3);

            std::vector<UV> uvs;
            uvs.reserve(mesh->mNumVertices);
            const bool has_uv = mesh->HasTextureCoords(0);
            for (auto i{0u}; i < mesh->mNumVertices; ++i)
            {
                if (has_uv)
                {
                    uvs.push_back({mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y});
                }
                else
                {
                    uvs.push_back({0.0f, 0.0f});
                }
            }

            // Vytvoríme vrcholy aktuálneho meshu a pridáme ich k celku
            auto current_vertices = vertices(positions, normals, uvs);
            all_vertices.insert(all_vertices.end(), std::make_move_iterator(current_vertices.begin()),
                                std::make_move_iterator(current_vertices.end()));

            // Indexy s posunom o vertex_offset!
            for (auto i{0u}; i < mesh->mNumFaces; ++i)
            {
                const auto& face = mesh->mFaces[i];
                for (auto j{0u}; j < face.mNumIndices; ++j)
                {
                    all_indices.push_back(face.mIndices[j] + vertex_offset);
                }
            }
        }

        log::debug("Loaded model {} with {} vertices and {} indices across {} meshes", model_file, all_vertices.size(),
                   all_indices.size(), scene->mNumMeshes);

        // 4. Uloženie celého zlúčeného batohu do cache
        const auto [it, inserted] =
            m_loaded_meshes.emplace(model_file, LoadedMeshData{std::move(all_vertices), std::move(all_indices)});

        return {it->second.vertices, it->second.indices};
    }

    MeshData MeshLoader::cube()
    {
        const auto loaded = m_loaded_meshes.find("cube");
        if (loaded != std::ranges::cend(m_loaded_meshes))
        {
            return {loaded->second.vertices, loaded->second.indices};
        }
        const Vector3 positions[] = {// Predná strana (+Z) - OK
                                     {-1.0f, -1.0f, 1.0f},
                                     {1.0f, -1.0f, 1.0f},
                                     {1.0f, 1.0f, 1.0f},
                                     {-1.0f, 1.0f, 1.0f},

                                     // Zadná strana (-Z) - OPRAVENÉ (bolo otočené dovnútra)
                                     {1.0f, -1.0f, -1.0f},
                                     {-1.0f, -1.0f, -1.0f},
                                     {-1.0f, 1.0f, -1.0f},
                                     {1.0f, 1.0f, -1.0f},

                                     // Ľavá strana (-X) - OK
                                     {-1.0f, -1.0f, -1.0f},
                                     {-1.0f, -1.0f, 1.0f},
                                     {-1.0f, 1.0f, 1.0f},
                                     {-1.0f, 1.0f, -1.0f},

                                     // Pravá strana (+X) - OPRAVENÉ (bolo otočené dovnútra)
                                     {1.0f, -1.0f, 1.0f},
                                     {1.0f, -1.0f, -1.0f},
                                     {1.0f, 1.0f, -1.0f},
                                     {1.0f, 1.0f, 1.0f},

                                     // Horná strana (+Y) - OK
                                     {-1.0f, 1.0f, 1.0f},
                                     {1.0f, 1.0f, 1.0f},
                                     {1.0f, 1.0f, -1.0f},
                                     {-1.0f, 1.0f, -1.0f},

                                     // Spodná strana (-Y) - OK (upravené poradie pre správnu orientáciu textúry)
                                     {-1.0f, -1.0f, -1.0f},
                                     {1.0f, -1.0f, -1.0f},
                                     {1.0f, -1.0f, 1.0f},
                                     {-1.0f, -1.0f, 1.0f}};

        const Vector3 normals[] = {{0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},
                                   {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
                                   {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
                                   {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},
                                   {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},
                                   {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};

        const UV uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0, 0.0f}, {1.0f, 0.0f},
                          {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
                          {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f},
                          {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

        const std::vector<std::uint32_t> indices = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                                                    8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                                                    16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
        const auto new_item =
            m_loaded_meshes.emplace("cube", LoadedMeshData{vertices(positions, normals, uvs), std::move(indices)});
        return {new_item.first->second.vertices, new_item.first->second.indices};
    }
} // namespace game