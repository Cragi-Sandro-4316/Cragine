#pragma once
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ComponentSignature.h"
#include "Ecs/Archetypes/Archetype.h"
#include "Ecs/Events/EventManager.h"
#include "utils/Logger.h"

namespace crg::ecs {


    class ComponentManager;
    class QueryManager;
    class Entity;
    class QueryManager;
    using ArchetypeID = uint32_t;


    class ArchetypeManager {
    public:
        ArchetypeManager(ComponentManager& componentManager) :
        m_componentManager(componentManager) {}

        // Adds a new entity to the archetype that matches the given signature
        template<typename... Components>
        std::pair<ArchetypeID, bool> addEntity(
            ComponentSignature signature,
            EventManager& eventManager,
            Entity entity,
            Components... components
        ) {
            auto [archID, isNew] = getExactArchetype(signature);

            auto& arch = m_archetypes[archID];

            auto& chunk = arch.getFreeChunk();

            auto entCompID = m_componentManager.getID<Entity>();

            auto* entityBuffer = (Entity*)chunk.getBuffer(entCompID);

            entityBuffer[chunk.m_entityCount] = entity;

            ([&](auto& component) {
                using Component = std::decay_t<decltype(component)>;
                auto compID = m_componentManager.getID<Component>();

                auto* buffer = (Component*)chunk.getBuffer(compID);

                buffer[chunk.m_entityCount] = component;

                eventManager.emit(
                    Added<Component>{
                        .entity = entity,
                        .component = component
                    }
                );
            }(components), ...);

            chunk.m_entityCount++;

            return {archID, isNew};
        }


        void removeEntity(Entity entity, ArchetypeID archID, EventManager& eventManager) {
            auto& arch = m_archetypes[archID];

            auto [chunkIndex, entityIndex] = arch.findEntity(entity);

            auto* chunk = arch.getChunks()[chunkIndex].get();

            chunk->m_entityCount--;
            for (auto& compID : arch.getSignature()) {
                auto funcs = m_componentManager.getFuncTable(compID);

                auto* buffer = chunk->getBuffer(compID);

                funcs.emitRemoved(buffer, entityIndex, eventManager, entity);

                funcs.remove(buffer, entityIndex, chunk->m_entityCount);
            }

        }

        template<typename Component>
        std::tuple<
            bool,
            Archetype*,
            ArchetypeID
        > addComponent(
            Entity entity,
            ArchetypeID srcArchID,
            Component component,
            EventManager& eventManager
        ) {
//             ComponentID newCompID = m_componentManager.getID<Component>();
//
//             ComponentSignature srcSignature = getSignature(srcArchID);
//             ComponentSignature dstSignature = srcSignature;
//             auto res = dstSignature.insert(newCompID);
//
//             auto [dstArchID, isNew] = getExactArchetype(dstSignature);
//
//             Archetype& srcArch = getArchetype(srcArchID);
//             Archetype& dstArch = getArchetype(dstArchID);
//
//             if (!res.second) {
//                 LOG_CORE_WARNING("Entity {} already has this component", entity.id);
//                 return { false, &srcArch, srcArchID };
//             }
//
//             auto entityData = srcArch.getEntity(entity);
//
//             auto funcs = m_componentManager.getFuncTable(newCompID);
//
//             std::pair<ComponentID, void*> newComponent;
//             newComponent.first = newCompID;
//             newComponent.second = &component;
//
//             funcs.emitAdded(eventManager, entity, newComponent.second);
//
//             entityData.emplace_back(newComponent);
//
//             dstArch.addEntity(entityData);
//
//             srcArch.removeEntity(entity);
//
//             return std::make_tuple( isNew, &dstArch, dstArchID );
            return {};
        }


        template<typename Component>
        std::tuple<
            bool,
            Archetype*,
            ArchetypeID
        > removeComponent(
            Entity entity,
            ArchetypeID srcArchID,
            EventManager& eventManager
        ) {
//             ComponentID removeCompID = m_componentManager.getID<Component>();
//
//             ComponentSignature srcSignature = getSignature(srcArchID);
//             ComponentSignature dstSignature = srcSignature;
//             auto res = dstSignature.erase(removeCompID);
//
//             auto [dstArchID, isNew] = getExactArchetype(dstSignature);
//
//             Archetype& srcArch = getArchetype(srcArchID);
//             Archetype& dstArch = getArchetype(dstArchID);
//
//             if (res == 0) {
//                 LOG_CORE_WARNING("Entity {} does not have component {} to remove", entity.id, removeCompID);
//                 return { false, &srcArch, srcArchID };
//             }
//
//             auto entityData = srcArch.getEntity(entity);
//
//             auto funcs = m_componentManager.getFuncTable(removeCompID);
//
//             for (size_t i = 0; i < entityData.size(); i++) {
//                 if (entityData[i].first == removeCompID) {
//                     funcs.emitRemoved(eventManager, entity, entityData[i].second);
//
//                     entityData[i] = entityData.back();
//                     break;
//                 }
//             }
//             entityData.pop_back();
//
//             dstArch.addEntity(entityData);
//
//             srcArch.removeEntity(entity);
//
//             return std::make_tuple( isNew, &dstArch, dstArchID );
            return {};
        }


        // Fetches the archetype that matches exactly the given signature
        // @return: an std::pair where
        // @first is the id of the archetype
        // @second bool that says whether a new archetype was created
        std::pair<ArchetypeID, bool> getExactArchetype(ComponentSignature signature);

        // Returns the component signature of the given archetype
        ComponentSignature getSignature(ArchetypeID archID);


        std::unordered_set<Archetype*> getArchetypes(
            ComponentSignature signature,
            ComponentSignature withFilters,
            ComponentSignature withoutFilters
        );

        Archetype& getArchetype(ArchetypeID archID) {
            return m_archetypes[archID];
        }

    private:

        // Given a component id, a list of indices pointing to the archetypes containing said component is returned
        std::unordered_map<
            ComponentID,
            std::unordered_set<ArchetypeID>
        > m_componentMap;

        // Maps a component signature to its archetype
        std::unordered_map<
            ComponentSignature,
            ArchetypeID,
            ComponentSignatureHash
        > m_archetypeIDs;

        // All the archetypes
        std::vector<Archetype> m_archetypes;

        ComponentManager& m_componentManager;

    };

}
