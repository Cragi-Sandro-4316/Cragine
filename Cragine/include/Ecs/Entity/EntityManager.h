#pragma once

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Ecs/Archetypes/ArchetypeManager.h"
#include "Entity.h"
#include "utils/Logger.h"
#include <cstdint>


namespace crg::ecs {

    class EntityManager {
    public:



        // Creates a new entity and returns its handle
        Entity newEntity() {
            EntityId id;
            EntityGeneration generation = 0;
            if (m_freeIds.empty()) {
                id = m_nextIndex;
                m_nextIndex++;
            }
            else {
                id = m_freeIds.back();
                m_freeIds.pop_back();
                generation = m_generations[id]; // Generation gets increased at despawn, not at spawn
            }

            auto x = m_entityIds.emplace(id);

            m_generations.emplace(id, generation);

            return Entity {
                .id = id,
                .generation = generation
            };
        }

        // Provided a valid handle, the entity is removed and the handle invalidated
        void removeEntity(Entity entity) {
            m_generations[entity.id]++;
            m_entityIds.erase(entity.id);
            m_freeIds.push_back(entity.id);
        }

        void assignArchetype(Entity entity, ArchetypeID archID) {

            if (!isValid(entity)) {

                LOG_CORE_ERROR("Entity archetype assign error: handle for entity: {} is invalid", entity.id);
                return;
            }

            m_archetypeMap[entity.id] = archID;

        }

        ArchetypeID getArchetype(Entity entity) {
            if (!isValid(entity)) {
                LOG_CORE_ERROR("entity getArchetype failed: handle for entity {} is invalid", entity.id);
                return UINT32_MAX;
            }

            return m_archetypeMap[entity.id];
        }



        bool isValid(Entity entity) {
            return
                m_entityIds.contains(entity.id) &&
                m_generations.at(entity.id) == entity.generation;
        }


    private:


        // All alive entity ids
        std::unordered_set<EntityId> m_entityIds;

        // Links an entity id to its generation
        std::unordered_map<EntityId, EntityGeneration> m_generations;

        // All unused ids to be reused
        std::vector<EntityId> m_freeIds;

        // Next entity index
        EntityId m_nextIndex = 0;


        // Links a given entity to its respective archetype
        std::unordered_map<EntityId, ArchetypeID> m_archetypeMap;
    };


}
