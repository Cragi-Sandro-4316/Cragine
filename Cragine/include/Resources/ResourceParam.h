#pragma once

#include "Resources/ResourceManager.h"
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
    struct SystemParam<Res<ResourceName>> {

        struct State {
            ResourceManager* manager;
        };

        static State init(World& world) {
            State state = State {
                &world.getResourceManager()
            };

            return state;
        }

        static Res<ResourceName> fetch(State* state, World& world) {
            Res<ResourceName> resource {
                *state->manager->template getResource<ResourceName>()
            };

            return resource;
        }

    };


    template<typename ResourceName>
    struct SystemParam<ResMut<ResourceName>> {

        struct State {
            ResourceManager* manager;
        };

        static State init(World& world) {
            State state = State {
                &world.getResourceManager()
            };

            return state;
        }

        static ResMut<ResourceName> fetch(State* state, World& world) {
            ResMut<ResourceName> resource {
                *state->manager->template getResource<ResourceName>()
            };

            return resource;
        }

    };

}
