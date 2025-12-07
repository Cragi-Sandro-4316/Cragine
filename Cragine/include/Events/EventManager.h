#pragma once

#include "utils/Logger.h"
#include <typeindex>
#include <unordered_map>
#include <vector>


namespace crg {

    struct EventQueueEntry {
        void* vec;
        void (*destroy)(void*) = nullptr;
        void (*clear)(void*) = nullptr;
        void (*push)(void*, const void*) = nullptr;
    };

    class EventManager {
    public:

        EventManager() {
            x = 2;
        };

        template<typename Event>
        void registerEvent() {
            auto* v = new std::vector<Event>();

            m_queues[typeid(Event)] = EventQueueEntry {
                .vec = v,
                .destroy = [](void* ptr) { delete static_cast<std::vector<Event>*>(ptr); },
                .clear = [](void* ptr) { static_cast<std::vector<Event>*>(ptr)->clear(); },
                .push = [](void* ptr, const void* data) {
                    static_cast<std::vector<Event>*>(ptr)->push_back(*static_cast<const Event*>(data));
                }
            };
        }

        template<typename Event>
        void emit(const Event& e) {
            LOG_CORE_TRACE("size: {}", m_queues.size());
            auto q = m_queues.find(typeid(Event));
            if (q == m_queues.end()) {
                std::type_index t = typeid(Event);
                LOG_CORE_WARNING("Event {} not registered, registering it now", t.name());
                registerEvent<Event>();
                q = m_queues.find(typeid(Event));
            }

            q->second.push(q->second.vec, &e);


        }

        template<typename Event>
        std::vector<Event>* read() {
            auto it = m_queues.find(typeid(Event));
            if (it == m_queues.end()) {
                std::type_index t = typeid(Event);
                LOG_CORE_WARNING("Event {} not registered, registering it now and returning empty group", t.name());
                registerEvent<Event>();
                return nullptr;
            }

            auto vec = static_cast<std::vector<Event>*>(it->second.vec);

            return vec;
        }

        void clearAll() {
            for (auto& [type, q] : m_queues) {
                q.clear(q.vec);
            }
        }

        ~EventManager() {
            for (auto& [type, q] : m_queues) {
                q.destroy(q.vec);
            }
        }

        int x;


        private:
        std::unordered_map<std::type_index, EventQueueEntry> m_queues{};
    };

}
