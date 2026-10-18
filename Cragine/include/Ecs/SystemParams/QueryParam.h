#pragma once

#include "Ecs/Archetypes/ComponentManager.h"
#include "Ecs/Query/CachedQuery.h"
#include "Ecs/Query/QueryFilters.h"
#include "Ecs/Query/QueryIterator.h"
#include "Ecs/Query/QueryManager.h"
#include "Ecs/Systems/SystemParam.h"
#include "Ecs/World.h"
#include "utils/Logger.h"

namespace crg::ecs {

    template<typename... Components>
    class Query {
    public:
        using FilteredTypes = filter_all<std::tuple<Components...>>::type;

        template<typename T>
        struct MakeIterator;

        template<typename... Cs>
        struct MakeIterator<std::tuple<Cs...>>{
            using type = QueryIterator<Cs...>;
        };

        using IteratorType = typename MakeIterator<FilteredTypes>::type;

        Query(CachedQuery& query, ComponentManager& componentManager) :
        query(query),
        componentManager(componentManager) {
            for (auto& chunk : query.m_chunks) {
                chunks.emplace_back(chunk);
            }
        }

        IteratorType begin() {
            ComponentSignature signature = componentManager.getSignature<Components...>();

            size_t chunk = 0;
            while(chunk < chunks.size() && chunks[chunk]->m_entityCount == 0) {
                ++chunk;
            }

            return IteratorType {
                .m_chunks = chunks,
                .m_componentManager = componentManager,
                .m_chunkIndex = 0,
                .m_entityIndex = 0
            };
        }

        IteratorType end() {
            ComponentSignature signature = componentManager.getSignature<Components...>();

            if (chunks.empty()) {
                LOG_CORE_WARNING("Query is empty");
                return begin();
            }

            size_t i = chunks.size() - 1;

            while (
                chunks[i]->m_entityCount == 0 &&
                i >= 0
            ) {
                if (i == 0) {
                    return begin();
                }

                i--;
            }

            return IteratorType {
                .m_chunks = chunks,
                .m_componentManager = componentManager,
                .m_chunkIndex = i + 1,
                .m_entityIndex = 0
            };
        }

        void update() {
            if (query.m_version == m_version) return;

            m_version = query.m_version;

            chunks = {};
            for (auto& chunk : query.m_chunks) {
                chunks.emplace_back(chunk);
            }
        }

    private:
        CachedQuery& query;
        ComponentManager& componentManager;

        std::vector<Chunk*> chunks;

        uint32_t m_version = 0;
    };

    template<typename... Components>
    class SystemParam<Query<Components...>> {
    public:

        struct State{
            QueryID id;
            Query<Components...> query;
        };

        static State init(World& world) {
            auto& queryManager = world.getQueryManager();

            auto& archetypeManager = world.getArchetypeManager();
            auto& componentManager = world.getComponentManager();

            auto id = queryManager.newQuery<Components...>(archetypeManager, componentManager);

            return State {
                .id = id,
                .query = Query<Components...>(queryManager.getQuery(id), componentManager)
            };
        }

        // Fetches the query from the world cache
        static Query<Components...>& fetch(State* state, World& world) {
            state->query.update();

            return state->query;
        }
    };
}
