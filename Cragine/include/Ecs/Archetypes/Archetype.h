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

        inline uint8_t* getBuffer(ComponentID compID) {
            return m_buffers.at(compID).get();
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
        m_entityCompID(componentManager.getID<Entity>()),
        m_signature(signature) {

            for (auto& compID : m_signature) {
                m_componentSizes[compID] = componentManager.getSize(compID);
            }

            m_chunks.emplace_back(std::make_unique<Chunk>(m_componentSizes));
        }

        inline std::vector<std::unique_ptr<Chunk>>& getChunks() {
            return m_chunks;
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


        ComponentSignature getSignature() {
            return m_signature;
        }

    private:

        const ComponentID m_entityCompID;

        ComponentSignature m_signature;

        // All chunks in this archetype
        std::vector<std::unique_ptr<Chunk>> m_chunks;

        std::unordered_map<ComponentID, size_t> m_componentSizes;
    };

}
