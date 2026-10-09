#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include "ComponentSignature.h"
#include "Ecs/Archetypes/ComponentManager.h"
#include "Ecs/Entity/Entity.h"

namespace crg::ecs {


    struct Chunk {
        static size_t constexpr MAX_ENTITY_COUNT = 64;

        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;

        Chunk(std::unordered_map<ComponentID, size_t>& sizes) {
            for (auto [compID, size] : sizes) {
                m_buffers[compID] = std::make_unique<uint8_t[]>(size * MAX_ENTITY_COUNT);
            }
        }

        std::unordered_map<
            ComponentID,
            std::unique_ptr<uint8_t[]>
        > m_buffers;

        uint32_t m_entityCount = 0;
    };



    class Archetype {
    public:

        Archetype(
            ComponentSignature& signature,
            ComponentManager& componentManager
        ) :
        m_signature(signature),
        m_entityCompID(componentManager.getID<Entity>()) {

            for (auto& compID : signature) {
                m_componentSizes[compID] = componentManager.getSize(compID);
                m_funcTables[compID] = componentManager.getFuncTable(compID);
            }

            m_chunks.emplace_back(std::make_unique<Chunk>(m_componentSizes));


        }

        void addEntity(
            std::vector<std::pair<
                ComponentID, void*
            >> componentData
        ) {
            auto& chunk = getFreeChunk();

            for (auto [compID, data] : componentData) {
                auto buffer = chunk.m_buffers[compID].get();
                auto funcs = m_funcTables[compID];

                funcs.arrWrite(buffer, data, chunk.m_entityCount);
            }

            chunk.m_entityCount++;
        }

        ComponentSignature getSignature() { return m_signature; }

        inline std::vector<std::unique_ptr<Chunk>>& getChunks() {
            return m_chunks;
        }

        void removeEntity(Entity& entity, EventManager* eventManager = nullptr) {
            auto [chunkID, entityID] = findEntity(entity);

            Chunk& chunk = *m_chunks[chunkID];

            for (auto compID : m_signature) {
                auto buffer = chunk.m_buffers[compID].get();
                auto funcs = m_funcTables[compID];

                if (eventManager) {
                    void* ptr = funcs.getPtr(buffer, entityID);

                    funcs.emitRemoved(*eventManager, entity, ptr);
                }

                funcs.swapAndPop(buffer, entityID, chunk.m_entityCount - 1);
            }

            chunk.m_entityCount--;
        }

        inline std::pair<size_t, size_t> findEntity(Entity& entity) {
            for (size_t i = 0; i < m_chunks.size(); i++) {
                Entity* entityBuff = reinterpret_cast<Entity*>(
                    m_chunks[i]->m_buffers[m_entityCompID].get()
                );

                for (size_t j = 0; j < m_chunks[i]->m_entityCount; j++) {
                    if (entity.id == entityBuff[j].id) {
                        return { i, j };
                    }
                }

            }

            return {};
        }

        inline Chunk& getFreeChunk() {
            for (auto& chunk : m_chunks) {
                if (chunk->m_entityCount < Chunk::MAX_ENTITY_COUNT) {
                    return *chunk;
                }
            }

            m_chunks.emplace_back(std::make_unique<Chunk>(m_componentSizes));

            return *m_chunks.back();
        }


        std::vector<std::pair<
            ComponentID, void*
        >> getEntity(Entity& entity) {
            auto [chunkID, entityID] = findEntity(entity);
            Chunk& chunk = *m_chunks[chunkID];

            std::vector<std::pair<
                ComponentID, void*
            >> result;

            for (auto& compID : m_signature) {

                auto funcs = m_funcTables[compID];

                std::pair<ComponentID, void*> elem;
                elem.first = compID;
                elem.second = funcs.getPtr(
                    chunk.m_buffers[compID].get(),
                    entityID
                );

                result.emplace_back(elem);
            }

            return result;
        }

    private:

        const ComponentID m_entityCompID;

        // All chunks in this archetype
        std::vector<std::unique_ptr<Chunk>> m_chunks;

        // The archetype's component signature
        ComponentSignature m_signature;

        std::unordered_map<ComponentID, FuncTable> m_funcTables;

        std::unordered_map<ComponentID, size_t> m_componentSizes;
    };

}
