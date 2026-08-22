#pragma once

#include "AssetManager/AssetManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshData.h"
#include "RenderModule/Transform.h"
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

    struct MeshMap {
        size_t chunk;
        size_t instance;
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
        m_meshMapBuffer(
            mapCount,
            BUFFER_TYPE(MeshMap),
            device,
            queue
        ) {
            m_meshChunks = std::vector<std::pair<MeshChunk, size_t>>(chunkCount);
            m_instanceData = std::vector<InstanceData>(instanceCount);
            m_meshMap.reserve(mapCount);
        }

        Handle<Mesh> loadMesh(const std::filesystem::path& path, Transform transform) {

            Handle<Mesh> handle = {std::hash<std::filesystem::path>{}(path)};

            LOG_CORE_INFO(
                "Transform Position [{}, {}, {}]",
                transform.translation.x,
                transform.translation.y,
                transform.translation.z
            );
            auto modelMatrix = transform.toMatrix();

            InstanceData instance {
                modelMatrix
            };

            LOG_CORE_INFO("Instance buffer index: {}", m_instanceCount);

            m_instanceData[m_instanceCount].modelMatrix = modelMatrix;
            m_instanceBuffer.write(instance, m_instanceCount);
            m_instanceCount++;

            if (m_meshIds.contains(handle.id)) {
                // LOG_CORE_INFO("Mesh {} already loaded", path.string());
                // Increase instances map

                auto& mesh = m_meshIds[handle.id];

                for (auto& chunkIdx : mesh.chunkIdxs) {
                    addInstance(chunkIdx);
                }

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

            m_meshIds.insert({
                handle.id,
                Mesh{
                    .chunkIdxs = std::vector<size_t>(chunkCount)
                }
            });

            std::vector<MeshChunk> chunks(chunkCount);

            for (size_t i = 0; i < chunks.size(); i++) {
                size_t chunkIdx = m_size + i;
                m_meshIds[handle.id].chunkIdxs[i] = chunkIdx;
                auto& gpuChunk = chunks[i];
                auto& cpuChunk = m_meshChunks[chunkIdx].first;

                addInstance(chunkIdx);

                for (size_t vertID = 0; vertID < CHUNK_VERTEX_COUNT; vertID++) {
                    size_t meshVertID = vertID + i * CHUNK_VERTEX_COUNT;

                    if (meshVertID < meshData.vertices.size()) {
                        gpuChunk.vertexData[vertID] = meshData.vertices[meshVertID];
                        cpuChunk.vertexData[vertID] = meshData.vertices[meshVertID];
                    }
                    else {
                        gpuChunk.vertexData[vertID] = meshData.vertices.back();
                        cpuChunk.vertexData[vertID] = meshData.vertices.back();
                    }
                }
            }
            LOG_CORE_INFO("msize: {}", m_size);

            m_chunkBuffer.writeBuffer(chunks.data(), chunkCount, m_size);

            m_size += chunkCount;

            return handle;
        }


        void unloadMesh(Handle<Mesh> handle) {
            auto it = m_meshIds.find(handle.id);

            if (it == m_meshIds.end()) {
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

        std::unordered_map<MeshID, Mesh> m_meshIds{};

        Buffer m_chunkBuffer;
        std::vector<
            std::pair<MeshChunk, size_t>
        > m_meshChunks;

        Buffer m_meshMapBuffer;
        std::vector<MeshMap> m_meshMap{};

        Buffer m_instanceBuffer;
        std::vector<InstanceData> m_instanceData{};
        size_t m_instanceCount = 0;

        void loadFromObj(const std::filesystem::path& path, MeshData& mesh);


        void addInstance(size_t chunkIdx) {
            auto chunkMapBack = chunkIdx + m_meshChunks[chunkIdx].second++;

            MeshMap map {
                .chunk = chunkIdx,
                .instance = m_instanceCount - 1
            };

            LOG_CORE_INFO("Map chunk: {}", map.chunk);
            LOG_CORE_INFO("Map instance: {}", map.instance);

            LOG_CORE_INFO("ChunkMap back: {}", chunkMapBack);
            m_meshMap.insert(
                m_meshMap.begin() + chunkMapBack,
                map
            );

            m_meshMapBuffer.write(map, chunkMapBack);
        }

    };



}
