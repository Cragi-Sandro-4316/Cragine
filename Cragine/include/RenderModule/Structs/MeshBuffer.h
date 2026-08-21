#pragma once

#include "AssetManager/AssetManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshData.h"
#include "utils/Logger.h"
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <vector>

namespace crg::renderer {

    constexpr size_t CHUNK_VERTEX_COUNT = 501;

    struct MeshChunk {
        VertexData vertexData[501];
    };

    struct InstanceData {
        alignas(16) mat4x4 modelMatrix;
    };


    struct Mesh {
        std::vector<size_t> chunkIdxs;
    };

    enum MeshBufferSize {
        Null,
        Small,
        Large
    };

    class MeshBuffer {
    public:

        using MeshID = size_t;

        MeshBuffer(
            wgpu::Device& device,
            wgpu::Queue& queue,
            size_t chunkCount,
            size_t instanceCount,
            size_t mapCount
        ) :
        m_chunkBuffer(
            chunkCount,
            BUFFER_TYPE(MeshChunk),
            device,
            queue
        ),
        m_instanceBuffer(
            instanceCount,
            BUFFER_TYPE(InstanceData),
            device,
            queue
        ),
        m_chunkInstanceMap(
            mapCount,
            BUFFER_TYPE(int),
            device,
            queue
        ) {}

        Handle<Mesh> loadMesh(const std::filesystem::path& path){

            Handle<Mesh> handle = {std::hash<std::filesystem::path>{}(path)};

            if (m_meshIds.contains(handle.id)) {
                LOG_CORE_INFO("Mesh {} already loaded", path.string());
                // Increase instances
                return handle;
            }

            MeshData meshData{};

            if (path.extension() == ".obj") {
                loadFromObj(path, meshData);
            }

            size_t chunkCount = std::ceil(
                (double)meshData.vertices.size() /
                (double)CHUNK_VERTEX_COUNT
            );


            if (m_size + chunkCount > m_chunkBuffer.size()) {
                LOG_CORE_ERROR("Mesh {} does not fit in mesh buffer", path.string());
                return handle;
            }

            m_meshIds[handle.id] = Mesh {
                std::vector<size_t>(chunkCount)
            };

            std::vector<MeshChunk> chunks(chunkCount);

            for (size_t i = 0; i < chunks.size(); i++) {
                auto& chunk = chunks[i];

                for (size_t vertID = 0; vertID < CHUNK_VERTEX_COUNT; vertID++) {
                    size_t meshVertID = vertID + i * CHUNK_VERTEX_COUNT;

                    if (meshVertID < meshData.vertices.size()) {
                        chunk.vertexData[vertID] = meshData.vertices[meshVertID];
                    }
                    else {
                        chunk.vertexData[vertID] = meshData.vertices.back();
                    }
                }
            }

            m_chunkBuffer.writeBuffer(chunks.data(), chunkCount, m_size);

            m_size += chunkCount;

            return handle;
        }


        void unloadMesh(Handle<Mesh> handle) {

        }

        inline size_t size() {
            return m_size;
        }

        inline size_t vertexCount() {
            return m_size * CHUNK_VERTEX_COUNT;
        }

        const Buffer& chunkBuffer() {
            return m_chunkBuffer;
        }

        const Buffer& instanceBuffer() {
            return m_instanceBuffer;
        }

        const Buffer& mapBuffer() {
            return m_chunkInstanceMap;
        }

    private:

        size_t m_size = 0;

        std::unordered_map<MeshID, Mesh> m_meshIds;

        Buffer m_chunkBuffer;

        Buffer m_chunkInstanceMap;

        Buffer m_instanceBuffer;

        void loadFromObj(const std::filesystem::path& path, MeshData& mesh);

    };



}
