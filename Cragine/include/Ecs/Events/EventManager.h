#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>


namespace crg::ecs {

    struct EventBuffer {
        std::shared_ptr<void> m_eventBuffer;
    };

    class EventManager {
    public:

        EventManager() {}

        template<typename Event>
        void registerEvent() {
            if (m_eventBuffers[0].contains(typeid(Event))) return;

            for (int i = 0; i < 2; i++) {

                m_eventBuffers[i][typeid(Event)] = EventBuffer{std::make_shared<std::vector<Event>>()};
            }

            m_clearBuffers[typeid(Event)] = [](void* buffer) {
                std::vector<Event>* tBuffer = static_cast<std::vector<Event>*>(buffer);
                tBuffer->clear();
            };

        }

        template<typename Event>
        void emit(const Event& e) {
            registerEvent<Event>();

            std::vector<Event>* eventBuffer =  static_cast<std::vector<Event>*>(
                m_eventBuffers[m_currentBuffer][typeid(Event)].m_eventBuffer.get()
            );

            eventBuffer->emplace_back(e);
        }

        template<typename Event>
        std::vector<Event>* read() {
            registerEvent<Event>();

            return static_cast<std::vector<Event>*>(
                m_eventBuffers[m_currentBuffer][typeid(Event)].m_eventBuffer.get()
            );
        }

        void clearAll() {

            for (auto& [type, buffer] : m_eventBuffers[m_currentBuffer]) {
                m_clearBuffers[type](buffer.m_eventBuffer.get());
            }

        }

        void swapBuffers() {
            m_currentBuffer = !m_currentBuffer;
        }

    private:

        using ClearBufferFunc = void(*)(void* buffer);

        std::unordered_map<
            std::type_index,
            EventBuffer
        > m_eventBuffers[2];

        std::unordered_map<
            std::type_index,
            ClearBufferFunc
        > m_clearBuffers;

        bool m_currentBuffer = 0;
    };

}
