#pragma once

#include "Ecs/Components/ComponentSignature.h"
#include "Ecs/World.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Components/QueryHelper.h"
#include "Ecs/Systems/SystemParam.h"

namespace crg::ecs {
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
