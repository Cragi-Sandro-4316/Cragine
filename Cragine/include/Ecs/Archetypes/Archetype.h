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


        Chunk(ComponentSignature& signature, ComponentManager& manager) {
            initBuffers(signature, manager);
        }

        // TODO: make more data oriented by allocating a single big array for all components and mapping indices and offsets to component ids.

        std::unordered_map<
            ComponentID,
            std::unique_ptr<uint8_t[]>
        > m_rawDataBuffers;

        uint32_t m_entityCount = 0;


        void initBuffers(ComponentSignature& signature, ComponentManager& compManager) {
            for (auto& compID : signature) {
                size_t componentSize = compManager.getSize(compID);

                m_rawDataBuffers[compID] = std::make_unique<uint8_t[]>(componentSize * MAX_ENTITY_COUNT);
            }
        }


        template<typename Component>
        void bufferInsert(Component value, ComponentManager& componentManager) {
            ComponentID compID = componentManager.getID<Component>();
            Component* buffer = (Component*)m_rawDataBuffers[compID].get();
            auto index = m_entityCount;

            buffer[index] = value;

        }


    };



    class Archetype {
    public:

        Archetype(
            ComponentSignature& signature,
            ComponentManager& componentManager
        ) :
        m_signature(signature),
        m_componentManager(componentManager) {

            m_chunks.emplace_back(std::make_unique<Chunk>(signature, m_componentManager));

            m_chunks.back()->initBuffers(signature, componentManager);

        }

        template<typename... Components>
        void addEntity(std::tuple<Components...> componentData) {
            auto& chunk = getFreeChunk();

            (chunk.bufferInsert(
                std::get<Components>(componentData),
                m_componentManager
            ), ...);


            chunk.m_entityCount++;
        }

        ComponentSignature getSignature() { return m_signature; }

        inline std::vector<std::unique_ptr<Chunk>>& getChunks() {
            return m_chunks;
        }


        void removeEntity(Entity& entity) {
            auto [chunkID, entityID] = findEntity(entity);

            Chunk& chunk = *m_chunks[chunkID];

            for (auto compID : m_signature) {
                auto buffer = chunk.m_rawDataBuffers[compID].get();
                auto& deleter = m_componentManager.getDeleter(compID);

                deleter(buffer, entityID, chunk.m_entityCount - 1);
            }

            chunk.m_entityCount--;
        }



        inline std::pair<size_t, size_t> findEntity(Entity& entity) {
            const auto entityCompID = m_componentManager.getID<Entity>();

            for (size_t i = 0; i < m_chunks.size(); i++) {
                Entity* entityBuff = reinterpret_cast<Entity*>(m_chunks[i]->m_rawDataBuffers[entityCompID].get());

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

            m_chunks.emplace_back(std::make_unique<Chunk>(m_signature, m_componentManager));

            return *m_chunks.back();
        }

    private:

        // All chunks in this archetype
        std::vector<std::unique_ptr<Chunk>> m_chunks;

        // The archetype's component signature
        ComponentSignature m_signature;

        ComponentManager& m_componentManager;






    };

}
