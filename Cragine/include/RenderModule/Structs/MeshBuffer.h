#pragma once

#include "AssetManager/AssetManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshData.h"
#include "utils/Logger.h"
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace crg::renderer {


    struct MeshChunk {
        VertexData vertexData[501];
        alignas(4) uint32_t instanceCount;
        alignas(4) uint32_t instanceIndex;
    };

    struct Instance {
        alignas(4) uint32_t vertexCount;
        alignas(4) uint32_t indexCount;
        alignas(16) mat4 modelMatrix;
    };


    struct Mesh {
        std::vector<size_t> chunkIdxs;
    };

    class MeshBuffer {
    public:

        MeshBuffer(
            Buffer meshChunkBuffer,
            Buffer instanceBuffer

        ) :
        m_meshBuffer(meshChunkBuffer),
        m_instanceBuffer(instanceBuffer) {}


        Handle<Mesh> loadMesh(const std::filesystem::path& path){

            Handle<Mesh> handle = {std::hash<std::filesystem::path>{}(path)};


            if (m_meshes.contains(handle.index)) {
                LOG_CORE_WARNING("Mesh already loaded. Skipping...");
                return handle;
            }

            MeshData meshData{};

            if (path.extension() == ".obj") {
                loadFromObj(path, meshData);
            }

            size_t chunkCount = std::ceil(meshData.vertices.size() / 501);

            if (m_size + chunkCount > m_meshBuffer.size()) {
                LOG_CORE_ERROR("Mesh {} does not fit in mesh buffer", path.string());
                return handle;
            }

            m_meshes[handle.index] = Mesh {
                .chunkIdxs = std::vector<size_t>(chunkCount)
            };

            std::vector<MeshChunk> chunks(chunkCount);


            for (size_t i = 0; i < chunks.size(); i++) {
                auto& chunk = chunks[i];

                for (size_t vertID = 0; vertID < 501; vertID++) {
                    size_t meshVertID = vertID + i * 501;

                    if (meshVertID < meshData.vertices.size()) {
                        chunk.vertexData[vertID] = meshData.vertices[meshVertID];
                    }
                    else {
                        chunk.vertexData[vertID] = meshData.vertices.back();
                    }
                }

                chunk.instanceCount = 1;
                chunk.instanceIndex = 1;
            }

            m_meshBuffer.writeBuffer(chunks, m_size);

            return handle;
        }

    private:

        size_t m_size = 0;

        std::unordered_map<size_t, Mesh> m_meshes;

        Buffer m_meshBuffer;

        Buffer m_instanceBuffer;


        void loadFromObj(const std::filesystem::path& path, MeshData& mesh);

    };



}
