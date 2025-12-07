#pragma once

namespace crg::ecs {

    class World;

    template<typename T>
    struct SystemParam {
        struct State{};

        static State init(World& world);
        static T fetch(State* state, World& world);

    };

}
