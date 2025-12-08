#pragma once

#include "Ecs/World.h"
#include "Ecs/Systems/SystemParam.h"
#include "Events/EventManager.h"
#include "utils/Logger.h"

namespace crg::ecs {


    template<typename EventName>
    class EventWriter {
    public:
        EventWriter(EventManager* manager) : m_manager(manager) {}
        EventWriter() = default;

        void write(EventName e) {
            m_manager->emit(e);
        }

        void clearAll() {
            m_manager->clearAll();
        }

    private:
        EventManager* m_manager = nullptr;
    };

    template<typename EventName>
    class EventReader {
        public:
            EventReader(EventManager* manager) : m_manager(manager) {}
            EventReader() = default;

            std::vector<EventName>& read() {
                 return *m_manager->read<EventName>();
            }

        private:
            EventManager* m_manager;
    };

    template<typename EventName>
    struct SystemParam<EventReader<EventName>> {

        struct State {
            // State(World* world) : m_world(world) {}
            World* m_world = nullptr;
        };

        static State init(World& world) {
            State state = State {
                &world
            };

            return state;
        }

        static EventReader<EventName> fetch(State* state, World& world) {
            if (!state->m_world) {
                LOG_CORE_WARNING("World is nullptr!");
               return EventReader<EventName>{};
            }

            auto manager = state->m_world->getEventManager();
            return EventReader<EventName> {
                manager
            };
        }

    };


    template<typename EventName>
    struct SystemParam<EventWriter<EventName>> {

        struct State {
            // State(World* world) : m_world(world) {}
            World* m_world = nullptr;
        };

        static State init(World& world) {
            State state = State {
                &world
            };

            return state;
        }

        static EventWriter<EventName> fetch(State* state, World& world) {
            if (!state->m_world) {
                LOG_CORE_WARNING("World is nullptr!");
               return EventWriter<EventName>{};
            }

            auto manager = state->m_world->getEventManager();

            auto returnVal = EventWriter<EventName> {
                manager
            };
            return returnVal;
        }

    };
}
