#pragma once

#include "AssetManager/AssetManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshData.h"
#include "RenderModule/Transform.h"
#include "glm/fwd.hpp"
#include "utils/Logger.h"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <vector>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    constexpr uint32_t CHUNK_VERTEX_COUNT = 501;

    struct MeshChunk {
        alignas(16)
        VertexData vertexData[CHUNK_VERTEX_COUNT];
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
            wgpu::BufferUsage::Storage  |
            wgpu::BufferUsage::MapRead  |
            wgpu::BufferUsage::MapWrite |
            wgpu::BufferUsage::CopyDst
        ),
        m_instanceBuffer(
            instanceCount,
            BUFFER_TYPE(InstanceData),
            device,
            queue,
            wgpu::BufferUsage::Storage  |
            wgpu::BufferUsage::MapRead  |
            wgpu::BufferUsage::MapWrite |
            wgpu::BufferUsage::CopyDst
        ),
        m_meshMapBuffer(
            mapCount,
            BUFFER_TYPE(ChunkMap),
            device,
            queue,
            wgpu::BufferUsage::Storage  |
            wgpu::BufferUsage::MapRead  |
            wgpu::BufferUsage::MapWrite |
            wgpu::BufferUsage::CopyDst
        ) {
            // m_meshChunks = std::vector<MeshChunk>(chunkCount);
            // m_instanceData = std::vector<InstanceData>(instanceCount);
            // m_meshMap.reserve(mapCount);
        }

        Handle<Mesh> loadMesh(const std::filesystem::path& path, Transform transform) {

            BufferView<MeshChunk> chunkBuffer = m_chunkBuffer.getBufferView<MeshChunk>();
            BufferView<InstanceData> instanceBuffer = m_instanceBuffer.getBufferView<InstanceData>();
            BufferView<ChunkMap> mapBuffer = m_meshMapBuffer.getBufferView<ChunkMap>();

            Handle<Mesh> handle = {std::hash<std::filesystem::path>{}(path)};
            auto modelMatrix = transform.toMatrix();

            InstanceData instance { modelMatrix };
            instanceBuffer[m_instanceCount++] = instance;

            auto it = m_meshChunkIdxs.find(handle.id);

            if (it != m_meshChunkIdxs.end()) {
                LOG_CORE_INFO("Mesh already loaded. Adding instance");
                auto& meshChunks = it->second;

                for (auto& chunkIdx : meshChunks.chunkIdxs) {

                    InstanceIndex& instanceIdx = m_chunkInstanceIdxs[chunkIdx];

                    ChunkMap map = ChunkMap {
                        .chunk = (uint32_t)chunkIdx,
                        .instance = (uint32_t) m_instanceCount - 1
                    };

                    size_t offset = instanceIdx.first + instanceIdx.count++;

                    mapBuffer.insert(&map, offset);
                    m_mapCount++;
                }

                return handle;
            }

            LOG_CORE_INFO("Spawning new mesh");
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

            size_t start = m_chunkCount;
            for (size_t i = 0; i < chunkCount; i++) {
                uint32_t chunkIndex = start + i;

                meshChunkIdxs.chunkIdxs[i] = chunkIndex;

                ChunkMap map = ChunkMap {
                    .chunk = chunkIndex,
                    .instance = static_cast<uint32_t>(m_instanceCount - 1)
                };

                mapBuffer[m_mapCount++] = map;

                m_chunkInstanceIdxs.emplace_back(InstanceIndex {
                    .first = static_cast<uint32_t>(m_mapCount - 1),
                    .count = 1
                });

                auto& meshChunk = chunkBuffer[chunkIndex];

                for (size_t j = 0; j < CHUNK_VERTEX_COUNT; j++) {
                    size_t vertexIndex = j + (i * CHUNK_VERTEX_COUNT);

                    if (vertexIndex < meshData.vertices.size()) {
                        meshChunk.vertexData[j] = meshData.vertices[vertexIndex];
                    }
                    else {
                        meshChunk.vertexData[j] = meshData.vertices.back();
                    }
                }

            }

            m_chunkCount += chunkCount;

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
                // auto& back = m_meshChunks[--m_size];

                // m_chunkBuffer.write(back, chunkIdx);
                // m_meshChunks[chunkIdx] = back;

                // auto& instanceIdx = m_chunkInstanceIdxs[chunkIdx];
                // auto& backInstanceIdx = m_chunkInstanceIdxs.back();

            }
        }

        inline size_t size() {
            return m_chunkCount;
        }

        inline size_t vertexCount() {
            return m_mapCount * CHUNK_VERTEX_COUNT;
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


        std::unordered_map<MeshID, Mesh> m_meshChunkIdxs{};

        Buffer m_chunkBuffer;
        size_t m_chunkCount = 0;
        // std::vector<MeshChunk> m_meshChunks;

        Buffer m_meshMapBuffer;
        size_t m_mapCount = 0;
        // std::vector<ChunkMap> m_meshMap{};

        Buffer m_instanceBuffer;
        // std::vector<InstanceData> m_instanceData{};
        size_t m_instanceCount = 0;

        // Maps a Chunk index to its first instance index and instance count
        std::vector<InstanceIndex> m_chunkInstanceIdxs;

        std::vector<size_t> m_freeInstanceIdxs;


        void loadFromObj(const std::filesystem::path& path, MeshData& mesh);

    };



}
