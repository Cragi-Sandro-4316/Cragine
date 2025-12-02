#pragma once

#include "../Components/ComponentSignature.h"
#include "Archetype.h"
#include "../Entities/Entity.h"
#include "../Entities/EntityManager.h"
#include "Ecs/Components/ComponentManager.h"
#include "utils/Logger.h"

#include <tuple>
#include <typeindex>
#include <unordered_map>

namespace crg::ecs {

    template<typename... T>
    class QueryResult;

    class ArchetypeManager {
    public:

    ArchetypeManager(EntityManager* entManager) :
    m_entityManager(entManager) {}

        // Returns a pointer to the archetype of the given entity
        Archetype* getEntityArchetype(Entity entity) {
            auto entitySignature = m_entityManager->getEntitySignature(entity);
            if (entitySignature.empty()) return nullptr;

            auto key = hashSignature(entitySignature);


            auto it = m_archetypeSignatures.find(key);  // Always returns m_archetypeSignatures.end()

            size_t index;

            // This is the problem
            if (it == m_archetypeSignatures.end()) {
                // Always enters here
                LOG_CORE_INFO("Created new archetype for entity id: {} generation: {}", entity.index, entity.generation);
                m_archetypes.emplace_back(entitySignature);
                m_archetypeSignatures[key] = m_archetypes.size() - 1; // Overwrites the right spot in the hash map; so the key is indeed correct.

                for (auto& component : entitySignature) {
                    m_componentLocations[component].emplace_back(m_archetypeSignatures.at(key));
                }

                index = m_archetypeSignatures.size() - 1;
            }
            else {
                index = it->second;
            }

            return &m_archetypes[index];
        }


        template<typename... Component>
        auto getComponentData(
            ComponentSignature infos,
            ComponentSignature withFilter,
            ComponentSignature withoutFilter
        ) {


            // DEBUG PRINT:
            for (auto info : infos) {
                LOG_CORE_TRACE("{}", info.type.name());
            }

            std::vector<const std::vector<size_t>*> archetypeLists;
            archetypeLists.reserve(sizeof...(Component));

            for (auto component : infos) {
                auto archetypes = m_componentLocations.find(component);
                if (archetypes == m_componentLocations.end()) {
                    LOG_CORE_WARNING("Query failure: Valid Archetype not found! Returning empty list");
                    auto returnLambda = [&]<typename... Ts>(std::tuple<Ts...>*) {
                        return QueryResult<Ts...>{};
                    };


                    return QueryResult<Component...>{};
                }

                archetypeLists.emplace_back(&archetypes->second);
            }

            std::sort(archetypeLists.begin(), archetypeLists.end(), [](auto a, auto b) {
                return a->size() < b->size();
            });

            auto archetypeIndices = intersectSortedVectors(archetypeLists);

            // Gets all data chunks
            std::vector<Chunk*> chunks;
            chunks.reserve(archetypeIndices.size());

            for (auto& index : archetypeIndices) {

                const auto& currentArchetype = m_archetypes[index];
                const ComponentSignature& archetypeTypes = currentArchetype.getTypes();

                // --- 1. WITHOUT Filter Check (Checks for FORBIDDEN components) ---
                bool hasForbiddenComponent = false;
                for (const auto& withoutInfo : withoutFilter) {
                    // If the archetype HAS a component from the withoutFilter list
                    if (std::find(archetypeTypes.begin(), archetypeTypes.end(), withoutInfo) != archetypeTypes.end()) {
                        hasForbiddenComponent = true;
                        break;
                    }
                }
                if (hasForbiddenComponent) {
                    continue; // Skip: It has a forbidden component
                }

                // --- 2. WITH Filter Check (Checks for REQUIRED components) ---
                // The variable should track if a required component is MISSING
                bool missingRequiredComponent = false;

                // CRITICAL FIX: Loop over withFilter, not withoutFilter!
                for (const auto& withInfo : withFilter) {
                    // If the archetype DOES NOT HAVE a component from the withFilter list
                    if (std::find(archetypeTypes.begin(), archetypeTypes.end(), withInfo) == archetypeTypes.end()) {
                        missingRequiredComponent = true;
                        break;
                    }
                }

                // CRITICAL FIX: Use the 'missing' flag to continue
                if (missingRequiredComponent) {
                    continue; // Skip: It is missing a required component
                }

                auto chunkList = m_archetypes[index].getChunks();

                for (auto& c : *chunkList) {
                    if (c.entityCount > 0) {
                        chunks.emplace_back(&c);
                    }
                }


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

            archetype->removeEntity(entity);
            m_entityManager->removeEntity(entity);
        }


        template<typename Component>
        void addComponent(Entity entityHandle, Component componentData) {
            auto signature = m_entityManager->getEntitySignature(entityHandle);
            auto it = m_archetypeSignatures.find(hashSignature(signature));
            if (it == m_archetypeSignatures.end()) {
                LOG_CORE_ERROR("Component add failed: Couldn't find archetype matching given entity's signature.");
                return;
            }

            auto& archetype = m_archetypes[it->second];

            auto entData = archetype.getRawCompData(entityHandle);

            uint8_t* newCompData = reinterpret_cast<uint8_t*> (&componentData);
            ComponentInfo newComponentInfo = ComponentInfo {
                .type = typeid(Component),
                .size = sizeof(Component),
                .alignment = alignof(Component),
                .copyFn = (std::is_trivially_copyable_v<Component>) ?
                [](void* dst, const void* src) {
                    std::memcpy(dst, src, sizeof(Component));
                } :
                [](void* dst, const void* src) {
                    new (dst) Component(*static_cast<const Component*>(src));
                }
            };

            entData.buffers.emplace_back(newCompData);
            entData.signatureIds[newComponentInfo] = entData.buffers.size() - 1;


            archetype.removeEntity(entityHandle);


            signature.emplace_back(ComponentInfo {
                .type = typeid(Component),
                .size = sizeof(Component),
                .alignment = alignof(Component),
                .copyFn = (std::is_trivially_copyable_v<Component>) ?
                [](void* dst, const void* src) {
                    std::memcpy(dst, src, sizeof(Component));
                } :
                [](void* dst, const void* src) {
                    new (dst) Component(*static_cast<const Component*>(src));
                }
            });

            std::sort(
                signature.begin(),
                signature.end(),
                [](const ComponentInfo& a, const ComponentInfo& b) {
                    return a.alignment > b.alignment;
                }
            );

            m_entityManager->updateEntitySignature(entityHandle, signature);


            auto newArchetype = getEntityArchetype(entityHandle);
            newArchetype->addFromRawData(entData, entityHandle);
        }


        template<typename Component>
        void removeComponent(Entity entityHandle) {
            auto signature = m_entityManager->getEntitySignature(entityHandle);
            auto it = m_archetypeSignatures.find(hashSignature(signature));
            if (it == m_archetypeSignatures.end()) {
                LOG_CORE_ERROR("Component removal error: Couldn't find archetype matching given entity's signature.");
                return;
            }

            auto& archetype = m_archetypes[it->second];

            auto entData = archetype.getRawCompData(entityHandle);

            auto componentInfo = ComponentInfo {
                .type = typeid(Component),
                .size = sizeof(Component),
                .alignment = alignof(Component),
                .copyFn = (std::is_trivially_copyable_v<Component>) ?
                [](void* dst, const void* src) {
                    std::memcpy(dst, src, sizeof(Component));
                } :
                [](void* dst, const void* src) {
                    new (dst) Component(*static_cast<const Component*>(src));
                }
            };


            auto removeCompIdx = entData.signatureIds.find(componentInfo);
            if (removeCompIdx == entData.signatureIds.end()) {
                LOG_CORE_WARNING("Component removal warning: given component not found in signature");
                return;
            }

            // TODO: Warning: not thread safe
            size_t lastIdx = entData.buffers.size() - 1;
            if (removeCompIdx->second != lastIdx) {
                entData.buffers[removeCompIdx->second] = entData.buffers[lastIdx];

                auto movedCompIt = std::find_if(
                    entData.signatureIds.begin(),
                    entData.signatureIds.end(),
                    [lastIdx](const auto& kv) {
                        return kv.second == lastIdx;
                    }
                );

                if (movedCompIt != entData.signatureIds.end()) {
                    movedCompIt->second = removeCompIdx->second;
                }
            }

            entData.buffers.pop_back();
            entData.signatureIds.erase(removeCompIdx);

            archetype.removeEntity(entityHandle);

            // Remove the component from the entity's signature
            auto sigIt = std::find_if(signature.begin(), signature.end(),
                [&componentInfo](const ComponentInfo& ci) { return ci == componentInfo; });

            if (sigIt != signature.end()) {
                *sigIt = signature.back();
                signature.pop_back();
            }

            std::sort(
                signature.begin(),
                signature.end(),
                [](const ComponentInfo& a, const ComponentInfo& b) {
                    return a.alignment > b.alignment;
                }
            );

            m_entityManager->updateEntitySignature(entityHandle, signature);

            auto newArchetype = getEntityArchetype(entityHandle);
            newArchetype->addFromRawData(entData, entityHandle);

        }


        void addEntityFromRawData(Entity entity, RawCompData rawData) {
            auto archetype = getEntityArchetype(entity);

            if (!archetype) {
                LOG_CORE_WARNING("No suitable archetype found");
            }

            // archetype->addEntity(data, entity);

            archetype->addFromRawData(rawData, entity);

            LOG_CORE_INFO("Entity successfully added to archetype");
        }


    private:

        using ComponentHash = size_t;

        ComponentHash hashSignature(ComponentSignature sig) {
            ComponentHash hash = 0;

            for (auto& comp : sig) {
                hash ^= comp.type.hash_code() + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }

        // Links a component signature with its archetype
        std::unordered_map<ComponentHash, size_t> m_archetypeSignatures{};

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
