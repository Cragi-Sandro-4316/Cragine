#pragma once

#include <cstddef>

#include "RenderModule/Handles.h"
#include "RenderModule/Structs/MeshCollection.h"

namespace crg::renderer {


    using MeshCollectionID = size_t;

    class MeshManager {
    public:


        Handle<MeshCollection> newCollection(
            wgpu::Device& device,
            wgpu::Queue& queue,
            MeshCollection::Size size = MeshCollection::Large
        ) {
            size_t chunkCount = 0;
            size_t instanceCount = 0;
            size_t mapCount = 0;

            switch (size) {
                case MeshCollection::Size::Large:
                    chunkCount = 2048;
                    instanceCount = 65535;
                    mapCount = 512000;
                break;
                case MeshCollection::Size::Small:
                    chunkCount = 1024;
                    instanceCount = 32767;
                    mapCount = 256000;
                break;
            }

            Handle<MeshCollection> handle {
                .id = m_nextID++
            };

            m_collections.emplace(
                handle.id,
                MeshCollection(
                    device,
                    queue,
                    chunkCount,
                    instanceCount,
                    mapCount
                )
            );

            return handle;
        }

        MeshCollection& getCollection(Handle<MeshCollection> handle) {
            return m_collections.at(handle.id);
        }

    private:

        MeshCollectionID m_nextID = 0;

        std::unordered_map<MeshCollectionID, MeshCollection> m_collections;

        std::unordered_map<MeshID, MeshCollectionID> m_meshLocations;
    };


}
