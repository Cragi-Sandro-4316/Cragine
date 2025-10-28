#pragma once

#include "utils/Logger.h"

#include <cstddef>
// #include <cstdint>
#include <typeindex>
#include <unordered_map>


namespace crg::ecs {

    struct ComponentInfo {
        std::type_index type;
        size_t size;
        size_t alignment;

        bool operator==(const ComponentInfo& other) const noexcept {
            return type == other.type;
        }
    };

    class ComponentRegistry {
    public:
        template<typename T>
        void registerComponent() {
            std::type_index type = typeid(T);
            if (m_componentInfos.contains(type)) return;

            m_componentInfos.emplace(std::type_index(typeid(T)), ComponentInfo {
                .type = type,
                .size = sizeof(T),
                .alignment = alignof(T)
            });

        }

        template<typename T>
        const ComponentInfo* getInfo() const {
            std::type_index type = typeid(T);

            auto it = m_componentInfos.find(type);
            if (it == m_componentInfos.end()) {
                LOG_CORE_ERROR("Component not registered: {}", type.name());
                return nullptr;
            }

            return &it->second;
        }

        const ComponentInfo* getInfo(std::type_index type) {
            auto it = m_componentInfos.find(type);
            if (it == m_componentInfos.end()) {
                LOG_CORE_ERROR("Component not registered: {}", type.name());
                return nullptr;
            }

            return &it->second;
        }

    private:
        // keeps track of the registered component's infos
        std::unordered_map<std::type_index, ComponentInfo> m_componentInfos{};

    };
}


namespace std {
    template<>
    struct hash<crg::ecs::ComponentInfo> {
        size_t operator()(const crg::ecs::ComponentInfo& component) const noexcept {
            return std::hash<std::type_index>()(component.type);
        }
    };
}
