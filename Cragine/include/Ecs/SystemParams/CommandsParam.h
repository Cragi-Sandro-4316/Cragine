#pragma once

#include "Ecs/Commands/Commands.h"
#include "Ecs/Ecs.h"

namespace crg::ecs {


    template<>
    class SystemParam<Commands> {
    public:
        struct State{
            Commands commands;
        };

        static State init(World& world) {
            return State{Commands(world)};
        }

        static Commands& fetch(State* state, World& world) {

            return state->commands;
        }
    };
}
