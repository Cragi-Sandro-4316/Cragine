#pragma once
#include "Ecs/Commands/Command.h"
#include "Ecs/World.h"
#include <tuple>
#include <utility>


namespace crg::ecs {
    class Commands  {
    public:
        Commands(World& world) : m_world(world), m_commandQueue(world.getCommandQueue()) {}

        template<typename... Components>
        void spawn(Components&&... components) {


            auto lambda = [
                components = std::make_tuple(std::forward<Components>(components)...)
            ](World& world) {
                std::apply([&world](auto&&...args) {
                    world.spawn(std::move(args)...);
                }, components);
            };

            m_commandQueue.emplace_back(Command(lambda));
        }


        void despawn(Entity entity) {
            auto lambda = [ent = entity](World& world) {
                world.despawn(ent);
            };

            m_commandQueue.emplace_back(Command(lambda));
        }

        template<typename Component>
        void addComponent(Entity entity, Component component) {

            auto lambda = [
                entity = entity,
                component = component
            ](World& world) {
                world.addComponent(entity, component);
            };

            m_commandQueue.emplace_back(Command(lambda));
        }


        template<typename Component>
        void removeComponent(Entity entity) {
            auto lambda = [ent = entity](World& world) {
                world.removeComponent<Component>(ent);
            };

            m_commandQueue.emplace_back(Command(lambda));
        }

        template<typename ResourceName, typename... Args>
        void registerResource(Args&&... args) {

            auto lambda = [
                args = std::make_tuple(std::forward<Args>(args)...)
            ](World& world) {
                std::apply([&world](auto&&...args) {
                    world.registerResource<ResourceName>(args...);
                }, args);
            };

            m_commandQueue.emplace_back(Command(lambda));
        }

    private:
        World& m_world;
        std::deque<Command>& m_commandQueue;
    };
}
