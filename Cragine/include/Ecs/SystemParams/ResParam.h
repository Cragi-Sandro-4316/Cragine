#pragma once

#include "Ecs/Resource/ResourceManager.h"
#include "Ecs/Systems/SystemParam.h"
#include "Ecs/World.h"

namespace crg::ecs {

    template<typename ResourceName>
    class Res {
    public:
        Res(ResourceName& resource) : m_resource(resource) {}

        // Gets the actual const reference
        const ResourceName& get() {
            return m_resource;
        }



    private:
        const ResourceName& m_resource;

    };



    template<typename ResourceName>
    class ResMut {
    public:
        ResMut(ResourceName& resource) : m_resource(resource) {}

        // Gets the actual reference
        ResourceName& get() {
            return m_resource;
        }



    private:
        ResourceName& m_resource;

    };

    template<typename ResourceName>
    class SystemParam<Res<ResourceName>> {
    public:
        struct State{

            ResourceManager* manager;
        };

        static State init(World& world) {
            return State{
                &world.getResourceManager()
            };
        }

        static Res<ResourceName>& fetch(State* state, World& world) {
            static Res<ResourceName> resource {
                *state->manager->template getResource<ResourceName>()
            };

            return resource;
        }
    };


    template<typename ResourceName>
    class SystemParam<ResMut<ResourceName>> {
    public:
        struct State{

            ResourceManager* manager;
        };

        static State init(World& world) {
            return State{&world.getResourceManager()};
        }

        static ResMut<ResourceName>& fetch(State* state, World& world) {
            static ResMut<ResourceName> resource {
                *state->manager->template getResource<ResourceName>()
            };
            return resource;
        }
    };

}
