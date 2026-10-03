#pragma once

#include "Ecs/Handle.h"

#include "RenderModule/Components/Mesh.h"
#include "RenderModule/Components/Transform.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshData.h"
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
        ) {}

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

            size_t instanceID = -1;
            if (m_freeInstanceIdxs.empty()) {
                instanceID = m_instanceCount++;
            }
            else {
                instanceID = m_freeInstanceIdxs.back();
                m_freeInstanceIdxs.pop_back();
            }

            auto it = m_meshClusterIDs.find(meshID);

            if (it != m_meshClusterIDs.end()) {
                LOG_CORE_INFO("Mesh {} already loaded", path.c_str());

                return Mesh {
                    .id = meshID,
                    .instanceID = instanceID,
                    .material = material
                };
            }

            MeshData meshData{};
            loadFromObj(path, meshData);

            size_t clusterCount = std::ceil(
                (double)meshData.vertices.size() /
                (double)CLUSTER_VERTEX_COUNT
            );

            m_meshClusterIDs.insert({
                meshID,
                ClusterIndices { .idxs = std::vector<uint32_t>(clusterCount) }
            });

            auto& indices = m_meshClusterIDs[meshID];

            fillClusters(
                meshID,
                collection,
                meshData,
                indices.idxs,
                clusterCount,
                textureHandle
            );
            m_clusterCount += clusterCount;

            return Mesh {
                .id = meshID,
                .instanceID = instanceID,
                .material = material
            };
        }


        void addInstance(
            Transform& transform,
            Mesh mesh
        ) {
            auto view = m_buffer.getBufferView();
            MeshCollectionData* collection = view.get();

            ClusterIndices clusterIndices = m_meshClusterIDs.at(mesh.id);

            auto modelMatrix = transform.toMatrix();

            Instance instance {
                .modelMatrix = modelMatrix
            };


            collection->instances[mesh.instanceID] = instance;

            for (auto& clusterIdx : clusterIndices.idxs) {
                if (m_instanceBlocks.size() <= clusterIdx) {
                    m_instanceBlocks.emplace_back(
                        InstanceBlock {
                            .first = (uint32_t) m_mapCount,
                            .count = 0
                        }
                    );
                }

                auto& instanceBlock = m_instanceBlocks[clusterIdx];

                ClusterInstance clusterInstance {
                    .cluster = (uint32_t) clusterIdx,
                    .instance = (uint32_t) mesh.instanceID
                };

                arrayInsert(
                    collection->clusterInstances,
                    MAX_CLUSTER_INSTANCE_MAP,
                    instanceBlock.first + instanceBlock.count++,
                    &clusterInstance
                );
                m_mapCount++;

                for (size_t i = clusterIdx + 1; i < m_instanceBlocks.size(); i++) {
                    m_instanceBlocks[i].first++;
                }
            }
        }


        void removeInstance(
            Mesh mesh
        ) {
            BufferView<MeshCollectionData> view = m_buffer.getBufferView();
            MeshCollectionData* collection = view.get();

            auto it = m_meshClusterIDs.find(mesh.id);
            if (it == m_meshClusterIDs.end()) {
                LOG_CORE_WARNING("Instance deletion: given handle not found. Skipping...");
                return;
            }

            size_t instanceOffset = -1;

            InstanceBlock& firstBlock = m_instanceBlocks[it->second.idxs[0]];
            for (size_t i = 0; i < firstBlock.count; i++) {
                if (collection->clusterInstances[i + firstBlock.first].instance == mesh.instanceID) {
                    instanceOffset = i;
                    break;
                }
            }

            auto freeInstance = collection->clusterInstances[firstBlock.first + instanceOffset].instance;
            m_freeInstanceIdxs.emplace_back(freeInstance);

            bool last = false;

            auto& clusterIdxs = it->second.idxs;
            for (auto& clusterIdx : clusterIdxs) {
                auto& instanceBlock = m_instanceBlocks[clusterIdx];

                size_t count = (MAX_CLUSTER_INSTANCE_MAP - instanceBlock.first + instanceOffset - 1) * sizeof(ClusterInstance);
                void* src = collection->clusterInstances + instanceBlock.first + instanceOffset + 1;
                void* dst = collection->clusterInstances + instanceBlock.first + instanceOffset;

                std::memmove(
                    dst,
                    src,
                    count
                );

                for (size_t i = clusterIdx; i < m_instanceBlocks.size(); i++) {
                    m_instanceBlocks[i].first--;
                }

                if (instanceBlock.count == 0) {
                    last = true;
                }
            }


            if (last) {
                LOG_CORE_INFO("Last mesh. Unloading...");
                // UnloadMesh();
            }

            m_mapCount -= clusterIdxs.size();
        }

        uint32_t vertexCount() {
            return m_clusterCount * CLUSTER_VERTEX_COUNT;
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


        std::vector<size_t> m_freeInstanceIdxs;

        Buffer<
            wgpu::BufferBindingType::Storage,
            MeshCollectionData
        > m_buffer;

        std::vector<InstanceBlock> m_instanceBlocks;

        void fillClusters(
            MeshID meshID,
            MeshCollectionData* collection,
            MeshData& data,
            std::vector<uint32_t>& indices,
            size_t& clusterCount,
            Handle<AtlasEntry>& textureHandle
        ) {
            size_t start = m_clusterCount;
            for(size_t i = 0; i < indices.size(); i++) {
                uint32_t clusterIndex = start + i;

                indices[i] = clusterIndex;
                m_clusterToMeshID[clusterIndex] = meshID;

                auto& cluster = collection->clusters[clusterIndex];
                cluster.textureIndex = textureHandle.id;

                for (size_t j = 0; j < CLUSTER_VERTEX_COUNT; j++) {
                    size_t vertexIndex = j + (i * CLUSTER_VERTEX_COUNT);

                    if (vertexIndex < data.vertices.size()) {
                        cluster.vertexData[j] = data.vertices[vertexIndex];
                    }
                    else {
                        cluster.vertexData[j] = data.vertices.back();
                    }
                }
            }
        }

        void loadFromObj(const std::filesystem::path& path, MeshData& meshData);

        void unloadMesh(
            Handle<Mesh> handle
        ) {
            BufferView<MeshCollectionData> view = m_buffer.getBufferView();
            MeshCollectionData* collection = view.get();

            ClusterIndices& clusterIDs = m_meshClusterIDs[handle.id];

            for (auto& clusterID : clusterIDs.idxs) {
                // Swap and pop
                VertexCluster& backCluster = collection->clusters[--m_clusterCount];
                collection->clusters[clusterID] = backCluster;

                // Find the mesh id of the back cluster
                auto& backMeshID = m_clusterToMeshID[m_clusterCount];

                // Update the moved chunk index in the mesh chunk list
                for (auto& cluster : m_meshClusterIDs[backMeshID].idxs) {
                    if (cluster == m_clusterCount) {
                        cluster = clusterID;
                        break;
                    }
                }

                m_clusterToMeshID[clusterID] = backMeshID;
                m_clusterToMeshID.erase(m_clusterCount);

                // Move instances
                InstanceBlock& backInstanceBlock = m_instanceBlocks.back();
                InstanceBlock& instanceBlock = m_instanceBlocks[clusterID];

                void* values = collection->clusterInstances + backInstanceBlock.first;

                size_t& index = instanceBlock.first;
                size_t& count = backInstanceBlock.count;

                std::memmove(
                    collection->clusterInstances + index + count,
                    collection->clusterInstances + index,
                    (MAX_CLUSTER_INSTANCE_MAP - index) * sizeof(ClusterInstance)
                );

                std::memcpy(
                    collection->clusterInstances + index,
                    values,
                    count * sizeof(ClusterInstance)
                );

                for (size_t i = instanceBlock.first; i < instanceBlock.first + backInstanceBlock.count; i++) {
                    collection->clusterInstances[i].cluster = clusterID;
                }

                instanceBlock = backInstanceBlock;
                m_instanceBlocks.pop_back();
            }

            m_meshClusterIDs.erase(handle.id);
        }


        // TODO: implement edge guards
        template<typename T>
        void arrayInsert(
            T* array,
            size_t arrSize,
            size_t index,
            T* values,
            size_t count = 1
        ) {
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
            std::memmove(
                array + index,
                array + index + count,
                (arrSize - index - count) * sizeof(T)
            );
        }


    };


}
