#include "Archetype.h"
#include "Ecs/Entities/Entity.h"
#include "Ecs/Entities/EntityManager.h"
#include "utils/Logger.h"
#include <cstdio>
#include <cstdlib>

namespace crg::ecs {

    // template<typename... Components>
    // void Archetype::addEntity(
    //     const std::tuple<Components...>& data,
    //     ComponentRegistry* compRegistry,
    //     Entity entityHandle
    // ) {

    //     if (!matchesArchetype<Components...>()) {
    //         LOG_CORE_ERROR("AddEntity Error: Type signature incorrect");
    //         return;
    //     }

    //     Chunk* chunk = findOrCreateChunk(compRegistry);


    //     // entityLocation.chunk = chunk;
    //     // entityLocation.archetype = this;
    //     // entityLocation.indexInChunk = chunk->entityCount;

    //     copyTupleToChunk(chunk, data, entityHandle);
    // }


    // template<typename... Components>
    // bool Archetype::matchesArchetype() const {
    //     if (sizeof...(Components) != m_types.size())
    //         return false;

    //     std::array<std::type_index, sizeof...(Components)> inputTypes = { typeid(Components)... };
    //     std::sort(inputTypes.begin(), inputTypes.end());

    //     for (size_t i = 0; i < inputTypes.size(); i++) {
    //         if (inputTypes[i] != m_types[i].type) return false;
    //     }

    //     return true;
    // }


    Chunk* Archetype::findOrCreateChunk(ComponentRegistry* registry) {
        Chunk* chunk;

        // 1. Find a chunk with space
        for (auto& c : m_chunks) {
            if (c.entityCount < c.MAX_CHUNK_CAPACITY) {
                return &c;
            }
        }

        // 2. If all chunks are full, make a new one
        m_chunks.emplace_back();
        chunk = &m_chunks.back();

        for (size_t i = 0; i < m_types.size(); i++) {
            size_t typeSize = m_types[i].size;

            // Allocate buffers
            chunk->componentBuffers.emplace_back(std::make_unique<uint8_t[]>(
                typeSize * chunk->MAX_CHUNK_CAPACITY
            ));

            // Push buffer indices
            chunk->componentIndices[m_types[i].type] = i;
        }

        return chunk;
    }

    // template<typename... Components>
    // void Archetype::copyTupleToChunk(Chunk* chunk, const std::tuple<Components...>& data, Entity entityHandle) {
    //     size_t entityIndex = chunk->entityCount;

    //     std::apply([&](
    //         const Components&... comps) {
    //             ((copySingleComponent(chunk, comps, typeid(Components), entityIndex)), ...);
    //         },
    //         data
    //     );

    //     chunk->entities[chunk->entityCount] = entityHandle;
    //     chunk->entityIndices[entityHandle] = chunk->entityCount;
    //     chunk->entityCount++;
    // }

    // template<typename T>
    // inline void Archetype::copySingleComponent(Chunk* chunk, const T& component, const std::type_index type, size_t entityIndex) {
    //     auto it = chunk->componentIndices.find(type);
    //     if (it == chunk->componentIndices.end()) {
    //         LOG_CORE_ERROR("Component copy error: Type not found in signature");
    //         return;
    //     }

    //     auto buffer = chunk->componentBuffers[it->second].get();
    //     std::memcpy(buffer + entityIndex * sizeof(T), &component, sizeof(T));
    // }

    void Archetype::removeEntity(Entity removeHandle, EntityManager* entManager) {

        // Find chunk
        size_t removeIndex = 0;
        Chunk* chunk = nullptr;
        for (auto& c : m_chunks) {
            auto it = c.entityIndices.find(removeHandle);
            if (it != c.entityIndices.end()) {
                chunk = &c;
                removeIndex = it->second;
                break;
            }
        }

        // If chunk not found, print error and return
        if (!chunk) {
            LOG_CORE_WARNING("Entity removal warning: Could not find given entity in this Archetype");
            return;
        }

        if (chunk->entityCount == 0) {
            LOG_CORE_WARNING("Entity removal warning: Attempted removal on empty chunk");
            return;
        }


        // Copy data for each component in the archetype
        for (size_t i = 0; i < m_types.size(); i++) {
            const auto& componentInfo = m_types[i];
            auto bufferIndex = chunk->componentIndices.find(componentInfo.type);
            if (bufferIndex == chunk->componentIndices.end()) {
                LOG_CORE_ERROR("Entity removal error: Component type not found in chunk type list.");
                return;
            }

            auto buffer = chunk->componentBuffers[bufferIndex->second].get();

            auto lastElement = buffer + (chunk->entityCount - 1) * componentInfo.size;

            auto removedElement = buffer + (removeIndex * componentInfo.size);

            std::memcpy(removedElement, lastElement, componentInfo.size);

        }

        // Find handle of last entity
        auto lastHandleIt = chunk->entities.find(chunk->entityCount - 1);
        if (lastHandleIt == chunk->entities.end()) {
            LOG_CORE_ERROR("Entity removal error: could not find last entity handle; index given not valid");
            return;
        }

        auto lastHandle = lastHandleIt->second;

        // Update bookkeeping
        chunk->entityIndices[lastHandle] = removeIndex;
        chunk->entities[removeIndex] = lastHandle;

        chunk->entities.erase(chunk->entityCount - 1);
        chunk->entityIndices.erase(removeHandle);

        // Decrease the entity counter
        chunk->entityCount--;
    }



}
