#include "mesh_loader.hpp"
#include "assimp/mesh.h"
#include "assimp/vector3.h"
#include "error.hpp"
#include "log.hpp"
#include "resource_loader.hpp"
#include "src/vector3.hpp"
#include "src/vertex_data.hpp"
#include "vector3.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

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
    MeshLoader::MeshLoader(ResourceLoader& resource_loader) : m_resource_loader(resource_loader) {}

    MeshData MeshLoader::load(std::string_view model_file, std::string_view model_name)
    {
        const auto model_file_data = m_resource_loader.load_binary(model_file);
        ensure(!model_file_data.empty(), "No model data!");
        ::Assimp::Importer importer{};
        const auto* scene =
            importer.ReadFileFromMemory(model_file_data.data(), model_file_data.size(),
                                        ::aiProcess_Triangulate | ::aiProcess_FlipUVs | ::aiProcess_CalcTangentSpace);
        ensure((scene != nullptr) && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "failed to load model {} {}",
               model_file, model_name);
        const auto loaded_meshes = std::span<::aiMesh*>{scene->mMeshes, scene->mNumMeshes};
        log::debug("Found meshes {}", loaded_meshes.size());
        for (const auto* mesh : loaded_meshes)
        {
            const auto loaded = m_loaded_meshes.find(mesh->mName.C_Str());
            if (loaded != std::ranges::cend(m_loaded_meshes))
            {
                continue;
            }
            const auto to_vector3 = [](const ::aiVector3D& v) { return Vector3{v.x, v.y, v.z}; };
            const auto positions = std::span<::aiVector3D>{mesh->mVertices, mesh->mVertices + mesh->mNumVertices} |
                                   std::views::transform(to_vector3);

            const auto normals = std::span<::aiVector3D>{mesh->mNormals, mesh->mNormals + mesh->mNumVertices} |
                                 std::views::transform(to_vector3);
            std::vector<UV> uvs{};
            std::vector<Vector3> tangents{};
            ensure(mesh->HasTextureCoords(0), "Mesh doesnt have tex coords");
            for (auto i{0u}; i < mesh->mNumVertices; ++i)
            {
                uvs.push_back({mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y});
                tangents.push_back({mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z});
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

            m_loaded_meshes.emplace(mesh->mName.C_Str(),
                                    LoadedMeshData{vertices(positions, normals,tangents, uvs), std::move(indices)});
        }

        const auto loaded = m_loaded_meshes.find(model_name);
        ensure(loaded != std::ranges::cend(m_loaded_meshes), "Failed to load {} from {}", model_name, model_file);
        return {loaded->second.vertices, loaded->second.indices};
    }
    MeshData MeshLoader::load(std::string_view model_file)
    {
        const auto loaded = m_loaded_meshes.find(model_file);
        if (loaded != std::ranges::cend(m_loaded_meshes))
        {
            return {loaded->second.vertices, loaded->second.indices};
        }

        const auto model_file_data = m_resource_loader.load_binary(model_file);
        ensure(!model_file_data.empty(), "No model data for file {}", model_file);

        ::Assimp::Importer importer{};
        const auto* scene = importer.ReadFileFromMemory(model_file_data.data(), model_file_data.size(),
                                                        ::aiProcess_Triangulate | ::aiProcess_CalcTangentSpace |
                                                            ::aiProcess_GenSmoothNormals |
                                                            ::aiProcess_PreTransformVertices | ::aiProcess_FlipUVs, "fbx"
                                                        );
        //::aiProcess_FlipUVs

        ensure((scene != nullptr) && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mNumMeshes > 0,
               "Failed to load model {}", model_file);

        std::vector<VertexData> all_vertices;
        std::vector<std::uint32_t> all_indices;

        for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
        {
            const auto* mesh = scene->mMeshes[m];

            const auto vertex_offset = static_cast<std::uint32_t>(all_vertices.size());

            const auto to_vector3 = [](const ::aiVector3D& v) { return Vector3{v.x, v.y, v.z}; };
            const auto positions = std::span<::aiVector3D>{mesh->mVertices, mesh->mVertices + mesh->mNumVertices} |
                                   std::views::transform(to_vector3);

            const auto normals = std::span<::aiVector3D>{mesh->mNormals, mesh->mNormals + mesh->mNumVertices} |
                                 std::views::transform(to_vector3);

            std::vector<UV> uvs;
            std::vector<Vector3> tangents;
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
                tangents.push_back({mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z});
            }

            auto current_vertices = vertices(positions, normals, tangents, uvs);
            all_vertices.insert(all_vertices.end(), std::make_move_iterator(current_vertices.begin()),
                                std::make_move_iterator(current_vertices.end()));

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
        const Vector3 positions[] = {
            {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, 1.0f},

            {1.0f, -1.0f, -1.0f},  {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},

            {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f},  {-1.0f, 1.0f, -1.0f},

            {1.0f, -1.0f, 1.0f},   {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},  {1.0f, 1.0f, 1.0f},

            {-1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, -1.0f},

            {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, 1.0f},  {-1.0f, -1.0f, 1.0f}};

        const Vector3 normals[] = {{0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},
                                   {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
                                   {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
                                   {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},
                                   {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},
                                   {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};


        const Vector3 tangents[] = {// Predná stena (+Z): U rastie pozdĺž +X
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    // Zadná stena (-Z): U rastie pozdĺž -X
                                    {-1.0f, 0.0f, 0.0f},
                                    {-1.0f, 0.0f, 0.0f},
                                    {-1.0f, 0.0f, 0.0f},
                                    {-1.0f, 0.0f, 0.0f},
                                    // Ľavá stena (-X): U rastie pozdĺž +Z
                                    {0.0f, 0.0f, 1.0f},
                                    {0.0f, 0.0f, 1.0f},
                                    {0.0f, 0.0f, 1.0f},
                                    {0.0f, 0.0f, 1.0f},
                                    // Pravá stena (+X): U rastie pozdĺž -Z
                                    {0.0f, 0.0f, -1.0f},
                                    {0.0f, 0.0f, -1.0f},
                                    {0.0f, 0.0f, -1.0f},
                                    {0.0f, 0.0f, -1.0f},
                                    // Horná stena (+Y): U rastie pozdĺž +X
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    // Spodná stena (-Y): U rastie pozdĺž +X
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f}};
        const UV uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0, 0.0f},    {1.0f, 0.0f},
                          {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
                          {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f},
                          {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

                    
        const std::vector<std::uint32_t> indices = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                                                    8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                                                    16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
        const auto new_item = m_loaded_meshes.emplace(
            "cube", LoadedMeshData{vertices(positions, normals, tangents, uvs), std::move(indices)});
        return {new_item.first->second.vertices, new_item.first->second.indices};
    }
    MeshData MeshLoader::plane()
    {
        const auto loaded = m_loaded_meshes.find("plane");
        if (loaded != std::ranges::cend(m_loaded_meshes))
        {
            return {loaded->second.vertices, loaded->second.indices};
        }
        const Vector3 positions[] = {{-1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, -1.0f}};
        const Vector3 normals[] = {// X,     Y,     Z
                                   {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
        const Vector3 tangents[] = {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
        const UV uvs[] = {

            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
        const auto new_item = m_loaded_meshes.emplace(
            "plane", LoadedMeshData{vertices(positions, normals, tangents, uvs), std::move(indices)});
        return {new_item.first->second.vertices, new_item.first->second.indices};
    }
    MeshData MeshLoader::sphere()
    {
        const auto loaded = m_loaded_meshes.find("sphere");
        if (loaded != std::ranges::cend(m_loaded_meshes))
        {
            return {loaded->second.vertices, loaded->second.indices};
        }

        static constexpr size_t xSegments = 36;
        static constexpr size_t ySegments = 18;
        constexpr float pi = std::numbers::pi_v<float>;

        std::vector<Vector3> positions;
        std::vector<Vector3> normals;
        std::vector<UV> uvs;
        std::vector<Vector3> tangents;
        std::vector<uint32_t> indices{};

        const size_t vertex_count = (xSegments + 1) * (ySegments + 1);
        positions.reserve(vertex_count);
        normals.reserve(vertex_count);
        tangents.reserve(vertex_count);
        uvs.reserve(vertex_count);
        
        indices.reserve(xSegments * ySegments * 6);

        for (size_t y = 0; y <= ySegments; ++y)
        {
            for (size_t x = 0; x <= xSegments; ++x)
            {
                float xSegment = static_cast<float>(x) / static_cast<float>(xSegments);
                float ySegment = static_cast<float>(y) / static_cast<float>(ySegments);

                float phi = xSegment * pi * 2.0f;
                float theta = ySegment * pi;

                float xPos = std::cos(phi) * std::sin(theta);
                float yPos = std::cos(theta);
                float zPos = std::sin(phi) * std::sin(theta);

                positions.emplace_back(xPos, yPos, zPos);
                normals.emplace_back(xPos, yPos, zPos);

                tangents.emplace_back(-std::sin(phi), 0.0f, std::cos(phi));

                uvs.emplace_back(xSegment, ySegment);
            }
        }

        for (size_t y = 0; y < ySegments; ++y)
        {
            for (size_t x = 0; x < xSegments; ++x)
            {
                uint32_t current = static_cast<uint32_t>(y * (xSegments + 1) + x);
                uint32_t next = static_cast<uint32_t>((y + 1) * (xSegments + 1) + x);

                indices.push_back(current);
                indices.push_back(current + 1);
                indices.push_back(next);

                indices.push_back(current + 1);
                indices.push_back(next + 1);
                indices.push_back(next);
            }
        }

        const auto new_item =
            m_loaded_meshes.emplace("sphere", LoadedMeshData{vertices(positions, normals, tangents, uvs), std::move(indices)});
        return {new_item.first->second.vertices, new_item.first->second.indices};
    }
} // namespace game