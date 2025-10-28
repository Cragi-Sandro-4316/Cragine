#pragma once

#include "Archetypes/ArchetypeManager.h"
#include "Components/ComponentManager.h"
#include "Components/ComponentSignature.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Entities/Entity.h"
#include "Entities/EntityManager.h"
#include <tuple>


namespace crg::ecs {
    class World {
    public:

    template<typename... Components>
    QueryResult<Components...> query() {
        auto result = m_archetypeManager.getComponentData<Components...>();

        return result;
    }

    template<typename... Components>
    void spawnEntity(std::tuple<Components...> data) {
        ComponentSignature compInfos = { ComponentInfo {
            .type = typeid(Components),
            .size = sizeof(Components),
            .alignment = alignof(Components)
        }... };

        auto handle = m_entityManager.newEntity(compInfos);

        m_archetypeManager.addEntity(handle, data);
    }

    void removeEntity(Entity entityHandle) {
        m_archetypeManager.removeEntity(entityHandle);
    }

    template<typename Component>
    void registerComponent() {
        m_componentRegistry.registerComponent<Component>();
    }

    private:

        // Holds component information, such as size, alignment and type id
        ComponentRegistry m_componentRegistry{};

        // Manages the world's entities
        EntityManager m_entityManager{};

        // The archetype manager
        ArchetypeManager m_archetypeManager{
            &m_entityManager,
            &m_componentRegistry
        };

    };
}
