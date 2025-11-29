#pragma once

#include "Archetypes/ArchetypeManager.h"
#include "Components/ComponentManager.h"
#include "Components/ComponentSignature.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Entities/Entity.h"
#include "Entities/EntityManager.h"
#include <cstring>
#include <tuple>
#include <type_traits>


namespace crg::ecs {
    class World {
    public:

    template<typename... Components>
    auto query() {
        auto result = m_archetypeManager.getComponentData<Components...>();

        return result;
    }

    template<typename Component>
    void addComponent(Entity entityHandle, Component data) {
        m_archetypeManager.addComponent<Component>(entityHandle, data);

    }

    template<typename Component>
    void removeComponent(Entity entityHandle) {
        m_archetypeManager.removeComponent<Component>(entityHandle);
    }


    template<typename... Components>
    void spawnEntity(std::tuple<Components...> data) {
        ComponentSignature compInfos = { ComponentInfo {
            .type = typeid(Components),
            .size = sizeof(Components),
            .alignment = alignof(Components),
            .copyFn = (std::is_trivially_copyable_v<Components>) ?
            [](void* dst, const void* src) {
                std::memcpy(dst, src, sizeof(Components));
            } :
            [](void* dst, const void* src) {
                new (dst) Components(*static_cast<const Components*>(src));
            }
        }..., {
            .type = typeid(Entity),
            .size = sizeof(Entity),
            .alignment = alignof(Entity),
            .copyFn = [](void* dst, const void* src) {
                std::memcpy(dst, src, sizeof(Entity));
            }
        }};

        auto handle = m_entityManager.newEntity(compInfos);

        auto dataWithEntity = std::tuple_cat(std::move(data), std::make_tuple(handle));

        m_archetypeManager.addEntity(handle, dataWithEntity);
    }

    void removeEntity(Entity entityHandle) {
        m_archetypeManager.removeEntity(entityHandle);
    }

    // template<typename Component>
    // void registerComponent() {
    //     m_componentRegistry.registerComponent<Component>();
    // }

    private:

        // Holds component information, such as size, alignment and type id
        // ComponentRegistry m_componentRegistry{};

        // Manages the world's entities
        EntityManager m_entityManager{};

        // The archetype manager
        ArchetypeManager m_archetypeManager{
            &m_entityManager,
            // &m_componentRegistry
        };

    };
}
