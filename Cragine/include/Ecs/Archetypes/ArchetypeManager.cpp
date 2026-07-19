#include "ArchetypeManager.h"
#include "Ecs/Archetypes/ComponentSignature.h"
#include <unordered_set>
#include <utility>
#include "Ecs/Query/QueryManager.h"
#include "Archetype.h"

namespace crg::ecs {


    std::pair<ArchetypeID, bool> ArchetypeManager::getExactArchetype(ComponentSignature signature) {
        auto it = m_archetypeIDs.find(signature);
        if (it != m_archetypeIDs.end()) {
            return { it->second, false };
        }

        ArchetypeID id = m_archetypes.size();

        m_archetypeIDs[signature] = id;

        for (auto& compID : signature) {
            m_componentMap[compID].emplace(id);
        }

        m_archetypes.emplace_back(Archetype{ signature, m_componentManager });


        return {id, true};
    }


    ComponentSignature ArchetypeManager::getSignature(ArchetypeID archID) {
        return m_archetypes[archID].getSignature();
    }

    std::unordered_set<Archetype*>  ArchetypeManager::getArchetypes(
        ComponentSignature signature,
        ComponentSignature withFilters,
        ComponentSignature withoutFilters
    ) {

        std::unordered_set<Archetype*> result;

        for (auto& compID : signature) {

            auto& archSet = m_componentMap[compID];

            for (auto& archID : archSet) {
                auto& arch = m_archetypes[archID];
                bool valid = true;
                for (auto& compID : signature) {
                    if (!arch.getSignature().contains(compID)) {
                        valid = false;
                    }
                }

                for (auto& compID : withFilters) {
                    if (!arch.getSignature().contains(compID)) {
                        valid = false;
                    }
                }

                for (auto& compID : withoutFilters) {
                    if (arch.getSignature().contains(compID)) {
                        valid = false;
                    }
                }

                if (valid) {
                    result.insert(&arch);
                }

            }

        }

        return result;
    }



}
