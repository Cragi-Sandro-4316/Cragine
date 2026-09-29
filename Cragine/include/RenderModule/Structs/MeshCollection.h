#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

#include "RenderModule/Components/Mesh.h"
#include "RenderModule/Components/Transform.h"
#include "RenderModule/Structs/MeshData.h"
#include "RenderModule/Handles.h"
#include "RenderModule/Structs/Buffer.h"

namespace crg::renderer {

    constexpr uint32_t CHUNK_VERTEX_COUNT = 501;

    using MeshID = size_t;

    struct MeshChunk {
        alignas(16)
        VertexData vertexData[CHUNK_VERTEX_COUNT];
        alignas(16)
        uint32_t textureIndex;
    };

    struct InstanceData {
        alignas(16) mat4x4 modelMatrix;
    };


    struct ChunkList {
        std::vector<size_t> chunkIdxs;
    };

    struct ChunkMap {
        uint32_t chunk;
        uint32_t instance;
    };



    // A Chunk's instance block
    struct InstanceBlock {
        uint32_t first;
        uint32_t count;
    };


    class MeshCollection {
    public:
        enum Size {
            Small,
            Large
        };

        MeshCollection(
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
        ) {}

        Handle<Mesh> loadMesh(const std::filesystem::path& path, Transform transform, Handle<AtlasEntry> textureHandle) {

            BufferView<MeshChunk> chunkBuffer = m_chunkBuffer.getBufferView<MeshChunk>();
            BufferView<InstanceData> instanceBuffer = m_instanceBuffer.getBufferView<InstanceData>();
            BufferView<ChunkMap> mapBuffer = m_meshMapBuffer.getBufferView<ChunkMap>();

            size_t handleId = std::hash<std::filesystem::path>{}(path);

            auto it = m_meshToChunkIdxs.find(handleId);

            if (it != m_meshToChunkIdxs.end()) {
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
                    .instanceID = instanceIndex
                };

                return handle;
            }

            MeshData meshData{};
            loadFromObj(path, meshData);

            size_t chunkCount = std::ceil(
                (double)meshData.vertices.size() /
                (double)CHUNK_VERTEX_COUNT
            );

            m_meshToChunkIdxs.insert({
                handleId,
                ChunkList { .chunkIdxs = std::vector<size_t>(chunkCount) }
            });

            auto& chunkIndices = m_meshToChunkIdxs[handleId];

            fillChunks(handleId, chunkCount, chunkIndices.chunkIdxs, chunkBuffer, textureHandle, meshData);
            m_chunkCount += chunkCount;

            size_t instanceIndex = addInstance(
                transform,
                chunkIndices.chunkIdxs,
                instanceBuffer,
                mapBuffer
            );

            Handle<Mesh> handle = {
                .id = handleId,
                .instanceID = instanceIndex
            };

            return handle;
        }


        void fillChunks(
            MeshID meshId, size_t chunkCount,
            std::vector<size_t>& chunkIndices,
            BufferView<MeshChunk>& chunkBuffer,
            Handle<AtlasEntry> textureHandle,
            MeshData& meshData
        ) {
            size_t start = m_chunkCount;
            for (size_t i = 0; i < chunkCount; i++) {
                uint32_t chunkIndex = start + i;

                chunkIndices[i] = chunkIndex;

                m_chunkToMeshID[chunkIndex] = meshId;

                // Fill chunks
                auto& meshChunk = chunkBuffer[chunkIndex];
                meshChunk.textureIndex = textureHandle.id;

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
                if (m_instanceBlocks.size() <= chunkIdx) {
                    m_instanceBlocks.emplace_back(
                        InstanceBlock {
                            .first = (uint32_t) m_mapCount,
                            .count = 0
                        }
                    );
                }

                auto& instanceBlock = m_instanceBlocks[chunkIdx];

                ChunkMap map {
                    .chunk = (uint32_t) chunkIdx,
                    .instance = (uint32_t) instanceIndex
                };

                mapBuffer.insert(&map, instanceBlock.first + instanceBlock.count++);
                m_mapCount++;

                for (size_t i = chunkIdx + 1; i < m_instanceBlocks.size(); i++) {
                    m_instanceBlocks[i].first++;
                }

            }

            return instanceIndex;
        }

        void deleteInstance(Handle<Mesh> handle) {
            auto it = m_meshToChunkIdxs.find(handle.id);

            if (it == m_meshToChunkIdxs.end()) {
                LOG_CORE_WARNING("Instance deletion: given handle not found. Skipping...");
                return;
            }

            BufferView<MeshChunk> chunkBuffer = m_chunkBuffer.getBufferView<MeshChunk>();
            BufferView<InstanceData> instanceBuffer = m_instanceBuffer.getBufferView<InstanceData>();
            BufferView<ChunkMap> mapBuffer = m_meshMapBuffer.getBufferView<ChunkMap>();

            // Find instance index
            size_t offset = -1;

            auto& firstBlock = m_instanceBlocks[it->second.chunkIdxs[0]];
            for (size_t i = 0; i < firstBlock.count; i++) {
                if (mapBuffer[i + firstBlock.first].instance == handle.instanceID) {
                    offset = i;
                    break;
                }
            }

            auto freeInstance = mapBuffer[firstBlock.first + offset].instance;
            m_freeInstanceIdxs.emplace_back(freeInstance);

            bool last = false;

            auto& chunkIdxs = it->second.chunkIdxs;
            for (auto& chunkIdx : chunkIdxs) {
                auto& instanceBlock = m_instanceBlocks[chunkIdx];

                mapBuffer.erase(instanceBlock.first + offset);

                instanceBlock.count--;

                for (size_t i = chunkIdx + 1; i < m_instanceBlocks.size(); i++) {
                    m_instanceBlocks[i].first--;
                }

                if (instanceBlock.count == 0) {
                    last = true;
                }
            }

            if (last) {
                unloadMesh(handle, chunkBuffer, mapBuffer);
            }

            m_mapCount -= chunkIdxs.size();
        }




        inline size_t size() {
            return m_chunkCount;
        }

        inline uint32_t vertexCount() {
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

        void bindLayoutEntry(std::vector<WGPUBindGroupLayoutEntry>& entries) {

            entries.emplace_back(WGPUBindGroupLayoutEntry {
                .nextInChain = nullptr,
                .binding = (uint32_t) entries.size(),
                .visibility = chunkBuffer().getStageVisibility(),
                .buffer = chunkBuffer().getBindingLayout()
            });

            entries.emplace_back(WGPUBindGroupLayoutEntry {
                .nextInChain = nullptr,
                .binding = (uint32_t) entries.size(),
                .visibility = instanceBuffer().getStageVisibility(),
                .buffer = instanceBuffer().getBindingLayout()
            });

            entries.emplace_back(WGPUBindGroupLayoutEntry {
                .nextInChain = nullptr,
                .binding = (uint32_t) entries.size(),
                .visibility = meshMapBuffer().getStageVisibility(),
                .buffer = meshMapBuffer().getBindingLayout()
            });
        }


        void bindEntry(std::vector<WGPUBindGroupEntry>& entries) {

            entries.emplace_back(WGPUBindGroupEntry{
                .nextInChain = nullptr,
                .binding = (uint32_t)entries.size(),
                .buffer = m_chunkBuffer.getRawHandle(),
                .offset = 0,
                .size = m_chunkBuffer.getByteSize(),
            });

            entries.emplace_back(WGPUBindGroupEntry{
                .nextInChain = nullptr,
                .binding = (uint32_t)entries.size(),
                .buffer = m_instanceBuffer.getRawHandle(),
                .offset = 0,
                .size = m_instanceBuffer.getByteSize(),
            });

            entries.emplace_back(WGPUBindGroupEntry{
                .nextInChain = nullptr,
                .binding = (uint32_t)entries.size(),
                .buffer = m_meshMapBuffer.getRawHandle(),
                .offset = 0,
                .size = m_meshMapBuffer.getByteSize(),
            });

        }

    private:

        // Maps a Mesh ID to its chunk index list
        std::unordered_map<MeshID, ChunkList> m_meshToChunkIdxs{};

        // Maps a Chunk index to the MeshID it belongs to
        std::unordered_map<size_t, MeshID> m_chunkToMeshID{};

        Buffer m_chunkBuffer;
        size_t m_chunkCount = 0;

        Buffer m_meshMapBuffer;
        size_t m_mapCount = 0;

        Buffer m_instanceBuffer;
        size_t m_instanceCount = 0;

        // Maps a Chunk index to its Instance block
        std::vector<InstanceBlock> m_instanceBlocks;

        std::vector<size_t> m_freeInstanceIdxs;


        void loadFromObj(const std::filesystem::path& path, MeshData& mesh);


        void unloadMesh(
            Handle<Mesh> handle,
            BufferView<MeshChunk>& chunkBuffer,
            BufferView<ChunkMap>& mapBuffer
        ) {
            // Chunk index list of the deleted mesh
            ChunkList& mesh = m_meshToChunkIdxs[handle.id];

            for (auto& chunkIdx : mesh.chunkIdxs) {
                // Swap and pop
                auto& backChunk = chunkBuffer[--m_chunkCount];
                chunkBuffer[chunkIdx] = backChunk;

                // Update maps...

                // find the back chunk's mesh id
                auto& backMeshID = m_chunkToMeshID[m_chunkCount];

                // Update the moved chunk index in the mesh chunk list
                for (auto& chunk : m_meshToChunkIdxs[backMeshID].chunkIdxs) {
                    if (chunk == m_chunkCount) {
                        chunk = chunkIdx;
                        break;
                    }
                }

                m_chunkToMeshID[chunkIdx] = backMeshID;
                m_chunkToMeshID.erase(m_chunkCount);

                // Move the instances
                auto& backInstanceBlock = m_instanceBlocks.back();

                auto& instanceBlock = m_instanceBlocks[chunkIdx];

                mapBuffer.insert(
                    mapBuffer.get() + backInstanceBlock.first,
                    instanceBlock.first,
                    backInstanceBlock.count
                );

                for (size_t i = instanceBlock.first; i < instanceBlock.first + backInstanceBlock.count; i++) {
                    mapBuffer[i].chunk = chunkIdx;
                }

                instanceBlock = backInstanceBlock;
                m_instanceBlocks.pop_back();
            }

            m_meshToChunkIdxs.erase(handle.id);
        }


        void printMap(BufferView<ChunkMap>& mapBuffer) {
            LOG_CORE_INFO("MapCount: {}", m_mapCount);
            for (int i = 0; i < m_mapCount; i++) {
                LOG_CORE_INFO("Map[{}]: (chunk {}, instance: {})", i, mapBuffer[i].chunk, mapBuffer[i].instance);
            }
        }

    };

}
