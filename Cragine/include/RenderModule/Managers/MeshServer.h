#pragma once

#include "RenderModule/Components/Mesh.h"
#include <filesystem>
#include <webgpu/webgpu.hpp>
#include "RenderModule/Handles.h"
#include "utils/Logger.h"

namespace crg::renderer {


    class MeshServer {
    public:

        Handle<Mesh> loadMesh(std::filesystem::path& path) {

            Handle<Mesh> handle {
                .id = m_currentID
            };

            Mesh mesh{};

            if (path.extension() == ".obj") {
                loadMeshFromObj(path, mesh);
            }

            m_meshes.insert({m_currentID, mesh});

            m_currentID++;

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
        size_t m_currentID = 0;

        std::unordered_map<size_t, Mesh> m_meshes;


        void loadMeshFromObj(std::filesystem::path& path, Mesh& mesh);

    };


}
