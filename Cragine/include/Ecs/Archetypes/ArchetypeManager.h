#pragma once

#include "../Components/ComponentSignature.h"
#include "Archetype.h"
#include "../Entities/Entity.h"
#include "../Entities/EntityManager.h"
#include "Ecs/Components/ComponentManager.h"
#include "Ecs/Components/QueryResult.h"
#include "utils/Logger.h"

#include <tuple>
#include <unordered_map>

namespace crg::ecs {
    class ArchetypeManager {
    public:

    ArchetypeManager(EntityManager* entManager) :
    m_entityManager(entManager) {}

        // Returns a pointer to the archetype of the given entity
        Archetype* getEntityArchetype(Entity entity) {
            auto entitySignature = m_entityManager->getEntitySignature(entity);
            if (entitySignature.empty()) return nullptr;


            if (!m_archetypeSignatures.contains(entitySignature)) {
                LOG_CORE_INFO("Created new archetype for entity id: {} generation: {}", entity.index, entity.generation);
                m_archetypes.emplace_back(entitySignature);
                m_archetypeSignatures[entitySignature] = m_archetypes.size() - 1;

                for (auto& component : entitySignature) {
                    m_componentLocations[component].emplace_back(m_archetypeSignatures.at(entitySignature));
                }
            }

            size_t index = m_archetypeSignatures.at(entitySignature);

            return &m_archetypes[index];
        }


        template <typename... Components>
        ComponentSignature createSignature() {
            ComponentSignature signature = {{
                ComponentInfo {
                    .type = typeid(Components),
                    .size = sizeof(Components),
                    .alignment = alignof(Components)
                }
            }...};
            // (signature.push_back(*m_componentRegistry->getInfo<Components>()), ...);
            return signature;
        }

        template<typename... Component>
        QueryResult<Component...> getComponentData() {
            ComponentSignature infos = createSignature<Component...>();

            std::vector<const std::vector<size_t>*> archetypeLists;
            archetypeLists.reserve(sizeof...(Component));

            for (auto component : infos) {
                auto archetypes = m_componentLocations.find(component);
                if (archetypes == m_componentLocations.end()) {
                    LOG_CORE_WARNING("Query failure: Valid Archetype not found! Returning empty list");
                    return {};
                }
                archetypeLists.emplace_back(&archetypes->second);
            }

            std::sort(archetypeLists.begin(), archetypeLists.end(), [](auto a, auto b) {
                return a->size() < b->size();
            });

            auto archetypeIndices = intersectSortedVectors(archetypeLists);

            // TODO: get component data from chunks

            // Gets all data chunks
            std::vector<std::vector<Chunk>*> chunks;
            chunks.reserve(archetypeIndices.size());
            for (auto& index : archetypeIndices) {
                chunks.emplace_back(m_archetypes[index].getChunks());
            }

            return QueryResult<Component...>{chunks};
        }


        template<typename... Components>
        void addEntity(Entity entity, std::tuple<Components...>& data) {
            auto archetype = getEntityArchetype(entity);

            if (!archetype) {
                LOG_CORE_WARNING("No suitable archetype found");
            }

            archetype->addEntity(data, entity);

            LOG_CORE_INFO("Entity successfully added to archetype");
        }

        void removeEntity(Entity entity) {
            auto archetype = getEntityArchetype(entity);
            archetype->removeEntity(entity, m_entityManager);
        }

    private:



        // Links a component signature with its archetype
        std::unordered_map<ComponentSignature, size_t> m_archetypeSignatures{};

        // Vector of archetypes
        std::vector<Archetype> m_archetypes{};

        // Links a component to the archetypes that contain it
        std::unordered_map<ComponentInfo, std::vector<size_t>> m_componentLocations{};



        EntityManager* m_entityManager;

        // ComponentRegistry* m_componentRegistry;


    private:

        // Gets the intersection of a list of vectors
        std::vector<size_t> intersectSortedVectors(std::vector<const std::vector<size_t>*> lists) {
            if (lists.empty()) return {};

            std::vector<size_t> result = *lists[0];

            std::sort(result.begin(), result.end());

            for (size_t i = 1; i < lists.size(); i++) {

                std::vector<size_t> temp;

                auto next = *lists[i];
                std::sort(next.begin(), next.end());

                std::set_intersection(
                    result.begin(), result.end(),
                    next.begin(), next.end(),
                    std::back_inserter(temp)
                );

                result = std::move(temp);

                if (result.empty()) break;
            }

            return result;
        }

    };



}
