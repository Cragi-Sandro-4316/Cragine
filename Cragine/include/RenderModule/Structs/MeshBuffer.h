#pragma once

#include "AssetManager/AssetManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshData.h"
#include "RenderModule/Transform.h"
#include "glm/fwd.hpp"
#include "utils/Logger.h"
#include <cstdint>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <vector>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    constexpr size_t CHUNK_VERTEX_COUNT = 501;

    struct MeshChunk {
        alignas(16) VertexData vertexData[CHUNK_VERTEX_COUNT];
    };

    struct InstanceData {
        alignas(16) mat4x4 modelMatrix;
    };


    struct Mesh {
        std::vector<size_t> chunkIdxs;
    };

    struct ChunkMap {
        uint32_t chunk;
        uint32_t instance;
    };

    enum MeshBufferSize {
        Null,
        Small,
        Large
    };


    struct InstanceIndex {
        uint32_t first;
        uint32_t count;
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
            queue,
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst
        ),
        m_instanceBuffer(
            instanceCount,
            BUFFER_TYPE(InstanceData),
            device,
            queue
        ),
        m_meshMapBuffer(
            mapCount,
            BUFFER_TYPE(ChunkMap),
            device,
            queue
        ) {
            m_meshChunks = std::vector<MeshChunk>(chunkCount);
            m_instanceData = std::vector<InstanceData>(instanceCount);
            m_meshMap.reserve(mapCount);
        }

        Handle<Mesh> loadMesh(const std::filesystem::path& path, Transform transform) {

            Handle<Mesh> handle = {std::hash<std::filesystem::path>{}(path)};
            auto modelMatrix = transform.toMatrix();

            InstanceData instance { modelMatrix };
            m_instanceData.push_back(instance);

            m_instanceBuffer.write(instance, m_instanceCount++);

            auto it = m_meshChunkIdxs.find(handle.id);

            if (it != m_meshChunkIdxs.end()) {
                LOG_CORE_WARNING("Mesh already loaded");
                auto& meshChunks = it->second;

                for (auto& chunkIdx : meshChunks.chunkIdxs) {

                    InstanceIndex& instanceIdx = m_chunkInstanceIdxs[chunkIdx];

                    ChunkMap map = ChunkMap {
                        .chunk = (uint32_t)chunkIdx,
                        .instance = (uint32_t) m_instanceCount - 1
                    };

                    size_t offset = instanceIdx.first + instanceIdx.count;

                    m_meshMap.insert(
                        m_meshMap.begin() + offset,
                        map
                    );

                    LOG_CORE_TRACE("Mesh Map buffer write on offset: {}, count: {}", offset, m_meshMap.size() - offset);
                    m_meshMapBuffer.writeBuffer(
                        m_meshMap.data() + offset,
                        m_meshMap.size() - offset,
                        offset
                    );
                }

            }

            MeshData meshData{};
            loadFromObj(path, meshData);

            size_t chunkCount = std::ceil(
                (double)meshData.vertices.size() /
                (double)CHUNK_VERTEX_COUNT
            );

            m_meshChunkIdxs.insert({
                handle.id,
                Mesh{ .chunkIdxs = std::vector<size_t>(chunkCount) }
            });

            auto& meshChunkIdxs = m_meshChunkIdxs[handle.id];

            size_t start = m_size;
            for (size_t i = 0; i < chunkCount; i++) {
                uint32_t chunkIndex = start + i;

                meshChunkIdxs.chunkIdxs[i] = chunkIndex;

                ChunkMap map = ChunkMap {
                    .chunk = chunkIndex,
                    .instance = static_cast<uint32_t>(m_instanceCount - 1)
                };

                m_meshMap.push_back(map);

                m_chunkInstanceIdxs.emplace_back(InstanceIndex {
                    .first = static_cast<uint32_t>(m_meshMap.size() - 1),
                    .count = 1
                });

                m_meshMapBuffer.write(map, m_meshMap.size() - 1);

                auto& meshChunk = m_meshChunks[chunkIndex];

                for (size_t vertexIndex = 0; vertexIndex < CHUNK_VERTEX_COUNT; vertexIndex++) {

                    if (vertexIndex < meshData.vertices.size()) {
                        meshChunk.vertexData[vertexIndex] = meshData.vertices[vertexIndex];
                    }
                    else {
                        meshChunk.vertexData[vertexIndex] = meshData.vertices.back();
                    }
                }

                m_chunkBuffer.write(meshChunk, chunkIndex);
            }

            m_size += chunkCount;

            return handle;
        }




        void unloadMesh(Handle<Mesh> handle) {
            auto it = m_meshChunkIdxs.find(handle.id);

            if (it == m_meshChunkIdxs.end()) {
                LOG_CORE_WARNING("Mesh unloading: given handle not found. Skipping...");
                return;
            }

            auto& mesh = it->second;
            for (auto& chunkIdx : mesh.chunkIdxs) {
                auto& back = m_meshChunks[--m_size];

                m_chunkBuffer.write(back, chunkIdx);
                m_meshChunks[chunkIdx] = back;
            }
        }

        inline size_t size() {
            return m_size;
        }

        inline size_t vertexCount() {
            return m_meshMap.size() * CHUNK_VERTEX_COUNT;
        }

        const Buffer& chunkBuffer() {
            return m_chunkBuffer;
        }

        const Buffer& instanceBuffer() {
            return m_instanceBuffer;
        }

        const Buffer& meshMapBuffer() {
            return m_meshMapBuffer;
        }

    private:

        size_t m_size = 0;

        std::unordered_map<MeshID, Mesh> m_meshChunkIdxs{};

        Buffer m_chunkBuffer;
        std::vector<MeshChunk> m_meshChunks;

        Buffer m_meshMapBuffer;
        std::vector<ChunkMap> m_meshMap{};

        Buffer m_instanceBuffer;
        std::vector<InstanceData> m_instanceData{};
        size_t m_instanceCount = 0;

        std::vector<InstanceIndex> m_chunkInstanceIdxs;



        void loadFromObj(const std::filesystem::path& path, MeshData& mesh);

    };



}
