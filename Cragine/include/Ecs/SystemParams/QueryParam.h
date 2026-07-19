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

            m_buffers.makeBuffers(componentManager, query.m_chunks, m_chunkEntityCounts);

        }

        IteratorType begin() {
            ComponentSignature signature = componentManager.getSignature<Components...>();

            return IteratorType {
                .m_buffers = m_buffers.buffers,
                .m_chunkEntityCounts = m_chunkEntityCounts,
                .m_chunkIndex = 0,
                .m_entityIndex = 0
            };
        }

        IteratorType end() {
            ComponentSignature signature = componentManager.getSignature<Components...>();

            if (query.m_chunks.empty()) {
                LOG_CORE_WARNING("Query is empty");

                return begin();
            }

            size_t i = m_chunkEntityCounts.size() - 1;

            while (
                *m_chunkEntityCounts[i] == 0 &&
                i >= 0
            ) {
                if (i == 0 ) {
                    return begin();
                }

                i--;
            }


            return IteratorType {
                .m_buffers = m_buffers.buffers,
                .m_chunkEntityCounts = m_chunkEntityCounts,
                .m_chunkIndex = i + 1,
                .m_entityIndex = 0
            };
        }


        void update() {
            if (query.m_version == m_version) return;

            m_version = query.m_version;

            m_buffers = {};
            m_chunkEntityCounts.clear();
            m_buffers.makeBuffers(componentManager, query.m_chunks, m_chunkEntityCounts);


        }

    private:
        CachedQuery& query;
        ComponentManager& componentManager;


        uint32_t m_version = 0;

        template<typename... Cs>
        struct Buffers;

        template<typename... Cs>
        struct Buffers<std::tuple<Cs...>>{

            std::tuple<
                std::vector<Cs*>...
            > buffers;


            void makeBuffers(
                ComponentManager& componentManager,
                std::unordered_set<Chunk*>& chunks,
                std::vector<const uint32_t*>& chunkEntityCounts
            ) {
                for (auto& chunk : chunks) {

                    (
                        extractBuffer(
                            std::get<std::vector<Cs*>>(buffers),
                            chunk,
                            componentManager
                        ),
                        ...
                    );

                    chunkEntityCounts.emplace_back(&chunk->m_entityCount);
                }
            }

            template<typename Component>
            void extractBuffer(std::vector<Component*>& bufferList, Chunk* chunk, ComponentManager& componentManager) {
                auto compID = componentManager.getID<Component>();

                bufferList.emplace_back(
                    (Component*)chunk->m_rawDataBuffers[compID].get()
                );
            }

        };

        Buffers<FilteredTypes> m_buffers;

        std::vector<const uint32_t*> m_chunkEntityCounts;
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
