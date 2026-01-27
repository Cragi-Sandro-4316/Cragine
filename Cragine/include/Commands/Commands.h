#pragma once

#include "Resources/ResourceManager.h"
#include "Ecs/Systems/SystemParam.h"
#include "Ecs/World.h"

namespace crg::ecs {

    class Commands {
    public:
        Commands(ecs::World& world) : m_world(world) {}

        // Gets the actual const reference
        ecs::World& get() {
            return m_world;
        }



    private:
        ecs::World& m_world;

    };

    template<>
    struct SystemParam<Commands> {

        struct State {
            ecs::World* m_world;
        };

        static State init(World& world) {
            State state = State {
                &world
            };

            return state;
        }

        static Commands fetch(State* state, World& world) {
            Commands commands {
                *state->m_world
            };

            return commands;
        }

    };

}
