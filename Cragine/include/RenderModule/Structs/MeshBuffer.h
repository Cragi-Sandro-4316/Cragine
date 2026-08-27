#pragma once

#include "RenderModule/Handles.h"
#include "AssetManager/AssetManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/BufferView.h"
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

            size_t handleId = std::hash<std::filesystem::path>{}(path);

            auto it = m_meshChunkIdxs.find(handleId);

            if (it != m_meshChunkIdxs.end()) {
                LOG_CORE_INFO("Mesh already loaded. Adding instance");
                auto& chunkIndices = it->second;

                size_t instanceIndex = addInstance(
                    transform,
                    chunkIndices.chunkIdxs,
                    instanceBuffer,
                    mapBuffer
                );

                Handle<Mesh> handle = {
                    .id = handleId,
                    .instanceId = instanceIndex
                };

                return handle;
            }

            LOG_CORE_INFO("Spawning new mesh");
            MeshData meshData{};
            loadFromObj(path, meshData);

            size_t chunkCount = std::ceil(
                (double)meshData.vertices.size() /
                (double)CHUNK_VERTEX_COUNT
            );

            LOG_CORE_TRACE("loading Chunk count: {}, size: {}", chunkCount, meshData.vertices.size());

            m_meshChunkIdxs.insert({
                handleId,
                Mesh{ .chunkIdxs = std::vector<size_t>(chunkCount) }
            });

            auto& chunkIndices = m_meshChunkIdxs[handleId];

            fillChunks(chunkCount, chunkIndices.chunkIdxs, chunkBuffer, meshData);
            m_chunkCount += chunkCount;

            size_t instanceIndex = addInstance(
                transform,
                chunkIndices.chunkIdxs,
                instanceBuffer,
                mapBuffer
            );

            Handle<Mesh> handle = {
                .id = handleId,
                .instanceId = instanceIndex
            };

            return handle;
        }


        void fillChunks(size_t chunkCount, std::vector<size_t>& chunkIndices, BufferView<MeshChunk>& chunkBuffer, MeshData& meshData) {
            size_t start = m_chunkCount;
            for (size_t i = 0; i < chunkCount; i++) {
                uint32_t chunkIndex = start + i;

                chunkIndices[i] = chunkIndex;

                // Fill chunks
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

        }

        size_t addInstance(
            Transform& transform,
            std::vector<size_t>& chunkIndices,
            BufferView<InstanceData>& instanceBuffer,
            BufferView<ChunkMap>& mapBuffer
        ) {
            auto modelMatrix = transform.toMatrix();
            InstanceData instance { modelMatrix };
            size_t instanceIndex = -1;

            if (m_freeInstanceIdxs.empty()) {
                instanceIndex = m_instanceCount++;
            }
            else {
                instanceIndex = m_freeInstanceIdxs.back();
                m_freeInstanceIdxs.pop_back();
            }

            instanceBuffer[instanceIndex] = instance;

            for (auto& chunkIdx : chunkIndices) {
                if (m_chunkInstanceIdxs.size() <= chunkIdx) {
                    m_chunkInstanceIdxs.emplace_back(
                        InstanceIndex {
                            .first = (uint32_t) m_mapCount,
                            .count = 0
                        }
                    );
                }

                auto& instanceLocation = m_chunkInstanceIdxs[chunkIdx];

                ChunkMap map {
                    .chunk = (uint32_t) chunkIdx,
                    .instance = (uint32_t) instanceIndex
                };

                mapBuffer.insert(&map, instanceLocation.first + instanceLocation.count++);
                m_mapCount++;

                for (size_t i = chunkIdx + 1; i < m_chunkInstanceIdxs.size(); i++) {
                    m_chunkInstanceIdxs[i].first++;
                }

            }

            return instanceIndex;
        }

        void deleteInstance(Handle<Mesh> handle) {
            auto it = m_meshChunkIdxs.find(handle.id);

            if (it == m_meshChunkIdxs.end()) {
                LOG_CORE_WARNING("Instance deletion: given handle not found. Skipping...");
                return;
            }

            BufferView<MeshChunk> chunkBuffer = m_chunkBuffer.getBufferView<MeshChunk>();
            BufferView<InstanceData> instanceBuffer = m_instanceBuffer.getBufferView<InstanceData>();
            BufferView<ChunkMap> mapBuffer = m_meshMapBuffer.getBufferView<ChunkMap>();

            // Find instance index
            size_t offset = -1;

            auto& firstLocation = m_chunkInstanceIdxs[it->second.chunkIdxs[0]];
            for (size_t i = 0; i < firstLocation.count; i++) {
                if (mapBuffer[i + firstLocation.first].instance == handle.instanceId) {
                    offset = i;
                    break;
                }
            }

            for (auto& chunkIdx : it->second.chunkIdxs) {
                auto& instanceLocation = m_chunkInstanceIdxs[chunkIdx];

                mapBuffer.erase(instanceLocation.first + offset);
                instanceLocation.count--;
                m_mapCount--;

                for (size_t i = chunkIdx + 1; i < m_chunkInstanceIdxs.size(); i++) {
                    m_chunkInstanceIdxs[i].first--;
                }
            }

        }

        void printMap(BufferView<ChunkMap>& mapBuffer) {
            for (int i = 0; i < m_mapCount; i++) {
                LOG_CORE_INFO("Map[{}]: (chunk {}, instance: {})", i, mapBuffer[i].chunk, mapBuffer[i].instance);
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

        Buffer m_meshMapBuffer;
        size_t m_mapCount = 0;

        Buffer m_instanceBuffer;
        size_t m_instanceCount = 0;

        // Maps a Chunk index to its first instance index and instance count
        std::vector<InstanceIndex> m_chunkInstanceIdxs;

        std::vector<size_t> m_freeInstanceIdxs;


        void loadFromObj(const std::filesystem::path& path, MeshData& mesh);

    };



}
