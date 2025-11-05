#pragma once

#include "../Components/ComponentSignature.h"
#include "Entity.h"
#include "utils/Logger.h"
#include <algorithm>
#include <cstdint>

namespace crg::ecs {
    class EntityManager {
    public:

        // Gets a new entity handle
        Entity newEntity(ComponentSignature signature) {

            // Sort the component signature
            std::sort(
                signature.begin(),
                signature.end(),
                [](const ComponentInfo& a, const ComponentInfo& b) {
                    return a.alignment > b.alignment;
                }
            );

            // Either get a free index from the free list, or create a new one
            Entity entity;
            if (!m_freeHandles.empty()) {
                entity = m_freeHandles.back();
                m_freeHandles.pop_back();
                entity.index = m_currentIndex;
            }
            else {
                entity = Entity {
                    .index = m_currentIndex,
                    .generation = 0
                };

            }

            m_currentIndex++;

            // Save the entity component signature
            m_entitySignatures[entity.index] = signature;

            m_entityHandles.emplace_back(entity);

            return entity;
        }

        // Returns the signature of the given Entity
        const ComponentSignature getEntitySignature(Entity entity) const {
            if (!m_entitySignatures.contains(entity.index)) {
                return ComponentSignature{};
            }

            return m_entitySignatures.at(entity.index);
        }

        // Removes the given entity from the active entity list
        // and pushes it to the recyclable handle list
        void removeEntity(Entity entity) {
            uint32_t entityIndex = entity.index;
            uint32_t lastIndex = static_cast<uint32_t>(m_entityHandles.size() - 1);

            if (entityIndex >= m_entityHandles.size()) {
                return;
            }

            // Get the entity currently at the back
            Entity lastEntity = m_entityHandles[lastIndex];

            // Move last entity into the removed slot (if not the same one)
            m_entityHandles[entityIndex] = lastEntity;

            // Update the moved entity's index to reflect its new position
            m_entityHandles[entityIndex].index = entityIndex;

            // Remove the last element
            m_entityHandles.pop_back();

            // Move the signature associated with the last entity into the removed slot
            m_entitySignatures[entityIndex] = m_entitySignatures.at(lastEntity.index);

            // Erase the signature for the removed entity
            m_entitySignatures.erase(lastEntity.index);

            // Increase the generation for the freed handle and add it to the free list
            entity.generation++;
            m_freeHandles.push_back(entity);

            m_currentIndex--;
        }

        bool isEntityValid(Entity entity) {
            if (entity.index >= m_entityHandles.size()) {
                return false;
            }
            return entity.generation == m_entityHandles[entity.index].generation;
        }

        EntityLocation* getEntityLocation(Entity entityHandle) {
            auto it = m_entityLocations.find(entityHandle);
            if (it == m_entityLocations.end()) {
                LOG_CORE_ERROR("GetEntityLocation: Cannot find entity location instance; Handle is probably invalid");
                return nullptr;
            }

            return &it->second;
        }

    private:

        std::unordered_map<uint32_t, ComponentSignature> m_entitySignatures{};

        // Keeps track of where an entity is located
        std::unordered_map<Entity, EntityLocation> m_entityLocations{};

        std::vector<Entity> m_entityHandles{};

        std::vector<Entity> m_freeHandles{};

        uint32_t m_currentIndex = 0;

    };
}
