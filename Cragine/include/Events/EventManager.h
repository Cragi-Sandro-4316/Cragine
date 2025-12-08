#pragma once

#include "utils/Logger.h"
#include <array>
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

        EventManager() {}

        template<typename Event>
        void registerEvent() {

            for (int i = 0; i < m_queues.size(); i++) {
                auto* v = new std::vector<Event>();

                m_queues[i][typeid(Event)] = EventQueueEntry {
                    .vec = v,
                    .destroy = [](void* ptr) { delete static_cast<std::vector<Event>*>(ptr); },
                    .clear = [](void* ptr) { static_cast<std::vector<Event>*>(ptr)->clear(); },
                    .push = [](void* ptr, const void* data) {
                        static_cast<std::vector<Event>*>(ptr)->push_back(*static_cast<const Event*>(data));
                    }
                };
            }

        }

        template<typename Event>
        void emit(const Event& e) {
            auto& queues = m_queues[m_currentWriteBufferIndex];

            auto q = queues.find(typeid(Event));
            if (q == queues.end()) {
                std::type_index t = typeid(Event);
                LOG_CORE_WARNING("Event {} not registered, registering it now", t.name());
                registerEvent<Event>();
                q = queues.find(typeid(Event));
            }

            q->second.push(q->second.vec, &e);
        }

        template<typename Event>
        std::vector<Event>* read() {
            static std::vector<Event> EMPTY;
            auto& queues = m_queues[!m_currentWriteBufferIndex];

            auto it = queues.find(typeid(Event));
            if (it == queues.end()) {
                std::type_index t = typeid(Event);
                LOG_CORE_WARNING("Event {} not registered, registering it now and returning empty group", t.name());
                registerEvent<Event>();
                return &EMPTY;
            }

            auto vec = static_cast<std::vector<Event>*>(it->second.vec);

            return vec;
        }

        void clearAll() {
            for (auto& [type, q] : m_queues[m_currentWriteBufferIndex]) {
                q.clear(q.vec);
            }
        }

        ~EventManager() {
            for (auto& [type, q] : m_queues[0]) {
                q.destroy(q.vec);
            }

            for (auto& [type, q] : m_queues[1]) {
                q.destroy(q.vec);
            }
        }


        void swapBuffers() {
            for (auto& [type, q] : m_queues[!m_currentWriteBufferIndex]) {
                q.clear(q.vec);
            }

            m_currentWriteBufferIndex = !m_currentWriteBufferIndex;
        }

        private:
        // Double event buffer
        std::array<
            std::unordered_map<std::type_index, EventQueueEntry>,
            2
        > m_queues{};

        bool m_currentWriteBufferIndex = 0;
    };

}
