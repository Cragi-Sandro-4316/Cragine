#pragma once

#include "Ecs/Components/ComponentSignature.h"
#include "Ecs/Components/QueryHelper.h"

namespace crg::ecs {

    class World;

    template<typename T>
    struct SystemParam {
        struct State{};

        static State init(World& world);
        static T fetch(State* state, World& world);

    };


    template<typename... Components>
    class QueryResult;

    template<typename... Components>
    class Query;


    template<typename... Components>
    struct SystemParam<Query<Components...>> {

        struct State {
            ComponentSignature components;
            ComponentSignature withFilter;
            ComponentSignature withoutFilter;

        };

        static State init(World& world) {
            State state;

            QueryHelper::createSignature<Components...>(
                state.components,
                state.withFilter,
                state.withoutFilter
            );

            return state;
        }

        static Query<Components...> fetch(State* state, World& world) {

            auto qState = typename Query<Components...>::State(
                state->components,
                state->withFilter,
                state->withoutFilter
            );

            return Query<Components...>(qState, world);
        }

    };


}
