#pragma once

#include "Ecs/Archetypes/ArchetypeManager.h"
#include "Ecs/Archetypes/ComponentManager.h"

#include "Ecs/Commands/Command.h"
#include "Ecs/Entity/EntityManager.h"

#include "Ecs/Events/EventManager.h"
#include "Ecs/Query/QueryManager.h"
#include "Ecs/Resource/ResourceManager.h"
#include "Ecs/Schedule.h"
#include <deque>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "Ecs/Systems/System.h"
#include "Systems/ISystem.h"
#include "utils/Logger.h"

namespace crg::ecs {

    class World {
    public:

        template<typename... Args>
        void addSystem(
            Schedule schedule,
            void(*func)(Args...)
        ) {
            System<Args...>* system = new System<Args...>(func, *this);
            m_systems[schedule].push_back(system);
        }

        void runSystems(Schedule schedule) {
            for (auto& system : m_systems[schedule]) {
                system->run(*this);
            }
        }

        ComponentManager& getComponentManager() {
            return m_componentManager;
        }

        ArchetypeManager& getArchetypeManager() {
            return m_archetypeManager;
        }


        QueryManager& getQueryManager() {
            return m_queryManager;
        }


        template<typename... Components>
        Entity spawn(Components... components) {

            LOG_CORE_INFO("Spawning entity...");

            Entity entity = m_entityManager.newEntity();

            auto signature = m_componentManager.getSignature<Components..., Entity>();

            std::tuple<Components..., Entity> componentData = std::make_tuple(std::forward<Components>(components)..., entity);

            auto [archID, isNew] = m_archetypeManager.addEntity(signature, componentData);

            m_entityManager.assignArchetype(entity, archID);


            if (isNew) {
                m_queryManager.updateQueries(&m_archetypeManager.getArchetype(archID));
            }

            return entity;
        }


        void despawn(Entity entity) {
            if (!m_entityManager.isValid(entity)) {
                return;
            }

            ArchetypeID archID = m_entityManager.getArchetype(entity);

            m_archetypeManager.removeEntity(entity, archID);

            m_entityManager.removeEntity(entity);

        }

        template<typename Component>
        void addComponent(Entity entity, Component component) {

            ArchetypeID srcArchID = m_entityManager.getArchetype(entity);

            auto [createdArch, newArch, newArchID] = m_archetypeManager.addComponent(
                entity,
                srcArchID,
                component
            );

            if (createdArch) {
                m_queryManager.updateQueries(newArch);
            }

            m_entityManager.assignArchetype(entity, newArchID);
        }


        template<typename Component>
        void removeComponent(Entity entity)  {

            ArchetypeID srcArchID = m_entityManager.getArchetype(entity);

            auto [isNew, newArch, newArchID] = m_archetypeManager.removeComponent<Component>(
                entity,
                srcArchID
            );

            if (isNew) {
                m_queryManager.updateQueries(newArch);
            }

            m_entityManager.assignArchetype(entity, newArchID);
        }

        std::deque<Command>& getCommandQueue() {
            return m_commandQueue;
        }


        void runCommands() {
            for (auto& command : m_commandQueue) {
                command.m_run(*this);
            }

            m_commandQueue.clear();
        }


        template<typename ResourceName, typename... Args>
        void registerResource(Args&&... args) {
            m_resourceManager.registerResource<ResourceName>(args...);
        }

        ResourceManager& getResourceManager() {
            return m_resourceManager;
        }

        EventManager* getEventManager() {
            return &m_eventManager;
        }

    private:

        std::unordered_map<
            Schedule,
            std::vector<ISystem*>
        > m_systems;

        EntityManager m_entityManager;

        QueryManager m_queryManager{};

        ResourceManager m_resourceManager{};

        ComponentManager m_componentManager{m_queryManager.getComponentMap()};

        ArchetypeManager m_archetypeManager{ m_componentManager };

        EventManager m_eventManager{};


        std::deque<Command> m_commandQueue;

    };

}
