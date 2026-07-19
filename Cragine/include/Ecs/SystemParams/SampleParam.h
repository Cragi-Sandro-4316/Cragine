#pragma once

#include "Ecs/Systems/SystemParam.h"
namespace crg {
    struct SampleParam  {
        int x = 67;
    };

    template<>
    class ecs::SystemParam<SampleParam> {
    public:
        struct State{

            int y = 0;
        };

        static State init(World& world) {
            return State{2};
        }

        static SampleParam& fetch(State* state, World& world) {
            static SampleParam x{state->y};

            return x;
        }
    };
}
