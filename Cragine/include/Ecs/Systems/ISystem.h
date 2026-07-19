#pragma once


namespace crg::ecs {
    class World;
    class ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void run(World& world) = 0;
    };
}
