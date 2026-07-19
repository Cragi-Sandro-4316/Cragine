#pragma once


namespace crg::ecs {

    class World;

    template<typename T>
    class SystemParam {
    public:
        struct State{};

        static State init(World& world);
        static T& fetch(World& world);
    };

}
