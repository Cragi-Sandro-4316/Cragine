#pragma once

#include "RenderModule/Structs/MeshData.h"
#include <filesystem>
#include <webgpu/webgpu.hpp>
#include "RenderModule/Handles.h"
#include "utils/Logger.h"

namespace crg::renderer {


    class MeshServer {
    public:

        Handle<Mesh> spawnMesh(std::filesystem::path& path) {

            size_t id = std::hash<std::string>{}(path.string());

            Handle<Mesh> handle {
                .id = id
            };

            if (!m_meshes.contains(id)) {
                Mesh mesh{};

                if (path.extension() == ".obj") {
                    loadMeshFromObj(path, mesh);
                }

                m_meshes[id] = mesh;
                m_instanceCounts[id] = 0;
            }

            m_instanceCounts[id]++;

            return handle;
        }


        Mesh* getMeshPtr(Handle<Mesh> handle) {
            auto it = m_meshes.find(handle.id);

            if (it == m_meshes.end()) {
                LOG_CORE_ERROR("Mesh error: given handle is invalid");
                return nullptr;
            }

            return &it->second;
        }

        inline bool validateHandle(Handle<Mesh> handle) {
            return m_meshes.contains(handle.id);
        }

        void deloadMesh(Handle<Mesh> handle) {
            if (!validateHandle(handle)) {
                LOG_CORE_WARNING("Sampler deletion error: given handle is invalid");
                return;
            }

            m_meshes.erase(handle.id);
        }

    private:

        std::unordered_map<size_t, Mesh> m_meshes;
        std::unordered_map<size_t, uint32_t> m_instanceCounts;


        void loadMeshFromObj(std::filesystem::path& path, Mesh& mesh);

    };


}
