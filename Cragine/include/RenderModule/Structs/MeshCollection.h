#pragma once

#include "Ecs/Handle.h"

#include "RenderModule/Components/Mesh.h"
#include "RenderModule/Components/Transform.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/BufferView.h"
#include "RenderModule/Structs/MeshData.h"
#include "glm/fwd.hpp"
#include "utils/Logger.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    constexpr uint32_t CLUSTER_VERTEX_COUNT = 501;

    struct VertexCluster {
        alignas(16) VertexData vertexData[CLUSTER_VERTEX_COUNT];
        alignas(16) uint32_t textureIndex;
    };

    struct Instance {
        alignas(16) mat4x4 modelMatrix;
    };

    struct ClusterInstance {
        uint32_t cluster;
        uint32_t instance;
    };

    struct ClusterIndices {
        std::vector<uint32_t> idxs;
    };

    using MeshID = size_t;

    constexpr size_t MAX_CLUSTER_COUNT = 2048;
    constexpr size_t MAX_INSTANCE_COUNT = 65535;
    constexpr size_t MAX_CLUSTER_INSTANCE_MAP = 512000;

    struct MeshCollectionData {
        alignas(16) VertexCluster clusters[MAX_CLUSTER_COUNT];
        alignas(16) Instance instances[MAX_INSTANCE_COUNT];
        alignas(16) ClusterInstance clusterInstances[MAX_CLUSTER_INSTANCE_MAP];
    };

    struct InstanceBlock {
        size_t first;
        size_t count;
    };

    // TODO: cleanup
    struct AtlasEntry;
    struct IMaterial;

    class MeshCollection {
    public:
        using Type = MeshCollectionData;
        static constexpr wgpu::BufferBindingType bindingType = wgpu::BufferBindingType::Storage;

        MeshCollection(RenderContext& renderContext) :
        m_buffer(
            renderContext,
            wgpu::BufferUsage::Storage  |
            wgpu::BufferUsage::MapRead  |
            wgpu::BufferUsage::MapWrite |
            wgpu::BufferUsage::CopyDst
        ),
        m_instanceBlocks(MAX_CLUSTER_COUNT) {}

        Mesh loadMesh(
            const std::filesystem::path path,
            Handle<IMaterial> material,
            Handle<AtlasEntry> textureHandle = Handle<AtlasEntry> {
                .id = 0
            }
        ) {
            BufferView<MeshCollectionData> view = m_buffer.getBufferView();
            MeshCollectionData* collection = view.get();

            MeshID meshID = std::hash<std::filesystem::path>{}(path);

            auto instanceID = m_instanceCounts[meshID]++;

            Mesh mesh {
                .id = std::hash<std::filesystem::path>{}(path),
                .instanceID = instanceID,
                .material = material
            };

            auto it = m_meshClusterIDs.find(meshID);

            if (it != m_meshClusterIDs.end()) {
                LOG_CORE_WARNING("Mesh already loaded. Skipping...");

                return mesh;
            }

            MeshData meshData{};
            loadFromObj(path, meshData);

            size_t clusterCount = std::ceil(
                (double)meshData.vertices.size() /
                (double)CLUSTER_VERTEX_COUNT
            );

            m_meshClusterIDs.emplace(
                mesh.id,
                ClusterIndices { std::vector<uint32_t>(clusterCount) }
            );

            auto& ids = m_meshClusterIDs[mesh.id].idxs;

            for (size_t i = 0; i < ids.size(); i++) {
                ids[i] = m_clusterCount++;
                m_clusterToMeshID[ids[i]] = mesh.id;

                VertexCluster& cluster = collection->clusters[ids[i]];
                cluster.textureIndex = textureHandle.id;

                for (size_t j = 0; j < CLUSTER_VERTEX_COUNT; j++) {
                    size_t vertexIndex = j + (i * CLUSTER_VERTEX_COUNT);

                    if (vertexIndex < meshData.vertices.size()) {
                        cluster.vertexData[j] = meshData.vertices[vertexIndex];
                    }
                    else {
                        cluster.vertexData[j] = meshData.vertices.back();
                    }
                }
            }

            return mesh;
        }

        void addInstance(
            Transform& transform,
            Mesh mesh
        ) {
            auto view = m_buffer.getBufferView();
            MeshCollectionData* collection = view.get();

            LOG_CORE_ERROR("Append instance: ");
            LOG_CORE_INFO("Map count: {}", m_mapCount);

            appendInstance(collection, mesh, transform);

            for (size_t i = 0; i < m_mapCount; i++) {
                LOG_CORE_TRACE("map[{}]: cluster: {}, instanceID: {}",
                    i,
                    collection->clusterInstances[i].cluster,
                    collection->clusterInstances[i].instance
                );
            }

            LOG_CORE_INFO("Map count: {}", m_mapCount);

        }


        void removeInstance(
            Mesh mesh
        ) {
            BufferView<MeshCollectionData> view = m_buffer.getBufferView();
            MeshCollectionData* collection = view.get();
            LOG_CORE_ERROR("Remove instance: ");

            LOG_CORE_INFO("Map count: {}", m_mapCount);

            for (size_t i = 0; i < m_mapCount; i++) {
                LOG_CORE_TRACE("map[{}]: cluster: {}, instanceID: {}",
                    i,
                    collection->clusterInstances[i].cluster,
                    collection->clusterInstances[i].instance
                );
            }

            deleteInstance(collection, mesh);

            for (size_t i = 0; i < m_mapCount; i++) {
                LOG_CORE_TRACE("map[{}]: cluster: {}, instanceID: {}",
                    i,
                    collection->clusterInstances[i].cluster,
                    collection->clusterInstances[i].instance
                );
            }
            LOG_CORE_INFO("Map count: {}", m_mapCount);
        }

        uint32_t vertexCount() {
            return m_mapCount * CLUSTER_VERTEX_COUNT;
        }

        wgpu::Buffer& getRawBuffer() {
            return m_buffer.getRawBuffer();
        }

    private:

        size_t m_clusterCount = 0;
        size_t m_mapCount = 0;
        size_t m_instanceCount = 0;

        // Maps a mesh ID with its cluster index list
        std::unordered_map<MeshID, ClusterIndices> m_meshClusterIDs;

        // Maps a cluster index to the meshID it belongs to
        std::unordered_map<size_t, MeshID> m_clusterToMeshID;

        std::unordered_map<MeshID, uint32_t> m_instanceCounts;

        std::vector<size_t> m_freeInstanceIdxs;

        Buffer<
            wgpu::BufferBindingType::Storage,
            MeshCollectionData
        > m_buffer;

        std::vector<InstanceBlock> m_instanceBlocks;

        void loadFromObj(const std::filesystem::path& path, MeshData& meshData);

        // TODO: implement edge guards
        template<typename T>
        void arrayInsert(
            T* array,
            size_t arrSize,
            size_t index,
            T* values,
            size_t count = 1
        ) {

            if (arrSize < index) {
                LOG_CORE_ERROR("Array insert underflow.");
                return;
            }

            std::memmove(
                array + index + count,
                array + index,
                (arrSize - index) * sizeof(T)
            );

            std::memcpy(array + index, values, count * sizeof(T));
        }

        template<typename T>
        void arrayErase(
            T* array,
            size_t arrSize,
            size_t index,
            size_t count
        ) {

            if (index > arrSize) {
                LOG_CORE_ERROR("arrayErase: index > arrSize, size_t would underflow below.");
                return;
            }

            if (count > arrSize - index) {
                LOG_CORE_ERROR("arrayErase: index + count > arrSize, size_t would underflow");
                return;
            }


            std::memmove(
                array + index,
                array + index + count,
                (arrSize - index - count) * sizeof(T)
            );
        }

        void appendInstance(MeshCollectionData* collection, Mesh& mesh, Transform& transform) {

            uint32_t instanceIndex = -1;
            if (m_freeInstanceIdxs.empty()) {
                instanceIndex = m_instanceCount++;
            }
            else  {
                instanceIndex = m_freeInstanceIdxs.back();
                m_freeInstanceIdxs.pop_back();
            }

            collection->instances[instanceIndex] = {
                .modelMatrix = transform.toMatrix()
            };

            ClusterIndices& idx = m_meshClusterIDs.at(mesh.id);

            for (auto id : idx.idxs) {

                InstanceBlock& instanceBlock = m_instanceBlocks[id];

                ClusterInstance map {
                    .cluster = id,
                    .instance = (uint32_t) instanceIndex
                };

                arrayInsert(collection->clusterInstances, MAX_CLUSTER_INSTANCE_MAP, instanceBlock.first + mesh.instanceID, &map);
                m_mapCount++;

                for (size_t i = id + 1; i < m_instanceBlocks.size(); i++) {
                    m_instanceBlocks[i].first++;
                }
            }
        }

        void deleteInstance(MeshCollectionData* collection, Mesh& mesh) {
            ClusterIndices& idx = m_meshClusterIDs.at(mesh.id);

            LOG_CORE_INFO("Mesh id: {}, instance id: {}", mesh.id, mesh.instanceID);

            for (auto id : idx.idxs) {
                InstanceBlock& instanceBlock = m_instanceBlocks[id];

                m_freeInstanceIdxs.emplace_back(
                    collection->clusterInstances[instanceBlock.first + mesh.instanceID].instance
                );

                LOG_CORE_INFO("free Instance index: {}", m_freeInstanceIdxs.back());

                arrayErase(collection->clusterInstances, MAX_CLUSTER_INSTANCE_MAP, instanceBlock.first + mesh.instanceID, 1);
                m_mapCount--;

                for (size_t i = id; i < m_instanceBlocks.size(); i++) {
                    m_instanceBlocks[i].first--;
                }
            }

            if (--m_instanceCounts[mesh.id] == 0) {
                LOG_CORE_WARNING("Deloading mesh");
                unloadMesh(collection, mesh);
            }
        }


        void unloadMesh(MeshCollectionData* collection, Mesh& mesh) {
            ClusterIndices& idx = m_meshClusterIDs.at(mesh.id);

            for (auto id : idx.idxs) {
                collection->clusters[id] = collection->clusters[--m_clusterCount];
                MeshID backMeshID = m_clusterToMeshID[m_clusterCount];

                InstanceBlock& backBlock = m_instanceBlocks[m_clusterCount];

                uint32_t& backCount = m_instanceCounts[backMeshID];
                uint32_t& count = m_instanceCounts[mesh.id];

                InstanceBlock& block = m_instanceBlocks[id];

                arrayErase(collection->clusterInstances, MAX_CLUSTER_INSTANCE_MAP, block.first, count);

                arrayInsert(
                    collection->clusterInstances,
                    MAX_CLUSTER_INSTANCE_MAP,
                    block.first,
                    &collection->clusterInstances[backBlock.first],
                    backCount
                );

                count = backCount;

                updateClusterMaps(id);
            }
        }

        void updateClusterMaps(uint32_t newIndex) {
            // MeshToCluster:
            // - find cluster index
            // - override it with the new one
            // ClusterToMesh:
            // - erase back cluster
            // - insert move cluster index
            MeshID backMeshID = m_clusterToMeshID[m_clusterCount];

            auto& backList = m_meshClusterIDs[backMeshID].idxs;

            for (auto& index : backList) {
                if (index == m_clusterCount) {
                    index = newIndex;
                }
            }

            m_clusterToMeshID[newIndex] = backMeshID;
            m_clusterToMeshID.erase(m_clusterCount);
        }

    };


}
