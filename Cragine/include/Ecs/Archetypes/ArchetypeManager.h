#pragma once
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ComponentSignature.h"
#include "Ecs/Archetypes/Archetype.h"
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
            std::tuple<Components...> componentData
        ) {
            auto [archID, isNew] = getExactArchetype(signature);

            auto& arch = m_archetypes[archID];

            arch.addEntity(componentData);

            return {archID, isNew};
        }


        void removeEntity(Entity entity, ArchetypeID archID) {
            auto& arch = m_archetypes[archID];

            arch.removeEntity(entity);
        }

        template<typename Component>
        std::tuple<
            bool,
            Archetype*,
            ArchetypeID
        > addComponent(
            Entity entity,
            ArchetypeID srcArchID,
            Component component
        ) {
            ComponentID newCompID = m_componentManager.getID<Component>();

            ComponentSignature srcSignature = getSignature(srcArchID);
            ComponentSignature dstSignature = srcSignature;
            auto res = dstSignature.insert(newCompID);

            auto [dstArchID, isNew] = getExactArchetype(dstSignature);

            Archetype& srcArch = getArchetype(srcArchID);
            Archetype& dstArch = getArchetype(dstArchID);

            if (!res.second) {
                LOG_CORE_WARNING("Entity {} already has this component", entity.id);
                return { false, &srcArch, srcArchID };
            }

            auto [srcChunkID, srcEntityIdx] = srcArch.findEntity(entity);
            auto& srcChunk = srcArch.getChunks()[srcChunkID];

            auto& dstChunk = dstArch.getFreeChunk();
            auto dstEntityIdx = dstChunk.m_entityCount;

            for (auto& compID : srcSignature) {
                auto& copy = m_componentManager.getCopy(compID);
                auto* srcBuffer = srcChunk->m_rawDataBuffers[compID].get();
                auto* dstBuffer = dstChunk.m_rawDataBuffers[compID].get();

                copy(
                    (void*)srcBuffer,
                    srcEntityIdx,
                    (void*)dstBuffer,
                    dstEntityIdx
                );
            }
            Component* compBuff = (Component*)dstChunk.m_rawDataBuffers[newCompID].get();
            compBuff[dstEntityIdx] = component;

            dstChunk.m_entityCount++;

            srcArch.removeEntity(entity);

            return std::make_tuple( isNew, &dstArch, dstArchID );
        }


        template<typename Component>
        std::tuple<
            bool,
            Archetype*,
            ArchetypeID
        > removeComponent(
            Entity entity,
            ArchetypeID srcArchID
        ) {
            ComponentID newCompID = m_componentManager.getID<Component>();

            ComponentSignature srcSignature = getSignature(srcArchID);
            ComponentSignature dstSignature = srcSignature;
            auto res = dstSignature.erase(newCompID);

            auto [dstArchID, isNew] = getExactArchetype(dstSignature);

            Archetype& srcArch = getArchetype(srcArchID);
            Archetype& dstArch = getArchetype(dstArchID);

            if (res == 0) {
                LOG_CORE_WARNING("Entity {} does not have component {} to remove", entity.id, newCompID);
                return { false, &srcArch, srcArchID };
            }

            auto [srcChunkID, srcEntityIdx] = srcArch.findEntity(entity);
            auto& srcChunk = srcArch.getChunks()[srcChunkID];

            auto& dstChunk = dstArch.getFreeChunk();
            auto dstEntityIdx = dstChunk.m_entityCount;

            for (auto& compID : dstSignature) {
                auto& copy = m_componentManager.getCopy(compID);
                auto* srcBuffer = srcChunk->m_rawDataBuffers[compID].get();
                auto* dstBuffer = dstChunk.m_rawDataBuffers[compID].get();

                copy(
                    (void*)srcBuffer,
                    srcEntityIdx,
                    (void*)dstBuffer,
                    dstEntityIdx
                );
            }

            dstChunk.m_entityCount++;

            srcArch.removeEntity(entity);

            return std::make_tuple( isNew, &dstArch, dstArchID );
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
