#pragma once

#include <glm/glm.hpp>
#include "./MeshHandle.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Components/QueryParam.h"
#include "../utils/Loader.h"
#include "Resources/ResourceParam.h"
#include "utils/Logger.h"
#include <unordered_map>
#include <vector>


namespace crg::renderer {


    struct VertexAttributes {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;
    };

    struct MeshData {
        // std::vector<glm::vec3> positions;
        // std::vector<glm::vec3> normals;
        // std::vector<glm::vec3> colors;
        // std::vector<glm::vec2> uvs;

        // inline void clear() {
        //     positions.clear();
        //     normals.clear();
        //     colors.clear();
        //     uvs.clear();
        // }

        // inline void resize(size_t size) {
        //     positions.resize(size);
        //     normals.resize(size);
        //     colors.resize(size);
        //     uvs.resize(size);
        // }

        // inline size_t size() {
        //     return positions.size();
        // }

        std::vector<VertexAttributes> attributes;

    };

    class MeshPool {
    public:

        MeshData* get(MeshHandle handle) {
            if (!m_meshIdx.contains(handle.path)){
                LOG_CORE_ERROR("Mesh not found");
                return nullptr;
            }

            return &m_meshes[handle.id];
        }

        void load(MeshHandle handle) {

            m_meshIdx[handle.path] = m_currentIndex;
            m_meshes.emplace_back(MeshData{});
            Loader::loadObj(RESOURCE_DIR + handle.path, m_meshes[m_currentIndex]);
            handle.id = m_currentIndex;

            m_currentIndex++;
        }

        const auto& meshes() const {
            return m_meshes;
        }

    private:

        uint32_t m_currentIndex = 0;

        std::unordered_map<std::filesystem::path, uint32_t> m_meshIdx;
        std::vector<MeshData> m_meshes;
    };

    inline void loadMeshes(
        ecs::Query<MeshHandle> loadingMeshes,
        ecs::ResMut<MeshPool> meshPoolRes
    ) {
        auto& meshPool = meshPoolRes.get();

        for (auto [mesh] : loadingMeshes.iter()) {
            if (mesh.isLoaded) continue;
            meshPool.load(mesh);
            mesh.isLoaded = true;
        }

    }

}
