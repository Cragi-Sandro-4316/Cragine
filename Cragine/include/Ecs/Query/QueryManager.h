#pragma once

#include "Ecs/Archetypes/ComponentSignature.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "CachedQuery.h"
#include "Ecs/Archetypes/ArchetypeManager.h"

namespace crg::ecs {
    class Archetype;

    using QueryID = uint32_t;

    class QueryManager {
    public:

        template<typename... Components>
        QueryID newQuery(ArchetypeManager& archManager, ComponentManager& componentManager)  {

            auto [
                signature,
                withFilters,
                withoutFilters
            ] = componentManager.unpackQueryTypes<Components...>();

            // find the archetypes and cache the query
            auto archetypes = archManager.getArchetypes(
                signature,
                withFilters,
                withoutFilters
            );

            m_queries.emplace_back(std::make_unique<CachedQuery>(
                signature,
                withFilters,
                withoutFilters,
                archetypes
            ));

            for (auto compID : signature) {
                auto& set = m_componentMap[compID];
                set.insert(m_queries.size() - 1);
            }

            return m_queries.size() - 1;
        }

        void updateQueries(Archetype* archetype)  {

            ComponentSignature archSignature = archetype->getSignature();

            std::vector<CachedQuery*> queries;

            bool valid = true;
            for (auto& query : m_queries) {
                for (auto compID : query->m_signature) {
                    if (!archSignature.contains(compID)) {
                        valid = false;
                    }
                }

                for (auto compID : query->m_withFilters) {
                    if (!archSignature.contains(compID)) {
                        valid = false;
                    }
                }

                for (auto compID : query->m_withoutFilters) {
                    if (archSignature.contains(compID)) {
                        valid = false;
                    }
                }

                if (valid) {
                    queries.emplace_back(query.get());
                }
            }

            for (auto& query : queries) {
                query->update(archetype);
            }
        }

        void addComponent(ComponentID compID) {
            m_componentMap[compID] = std::unordered_set<QueryID>();
        }


        const std::unordered_set<QueryID>& getQuerySet(ComponentID compID)  {
            return m_componentMap[compID];
        }

        inline CachedQuery& getQuery(QueryID id)  {
            return *m_queries[id];
        }

        auto* getComponentMap() {
            return &m_componentMap;
        }

    private:

        std::vector<
            std::unique_ptr<CachedQuery>
        > m_queries;

        // std::vector<uint32_t> m_queryVersions;

        std::unordered_map<
            ComponentID,
            std::unordered_set<QueryID>
        > m_componentMap;

    };

}
