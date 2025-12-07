#pragma once

#include "Ecs/Systems/System.h"
#include "utils/Logger.h"
#include <memory>
#include <vector>


namespace crg::ecs {

    enum Schedule : int {
        Startup,
        Update,
        FixedUpdate
    };

    class SystemScheduler {
        public:
        SystemScheduler(World& w) : m_world(w) {}

        template <typename R, typename... Args>
        void addSystem(
            const Schedule schedule,
            R(*system)(Args...)
        ) {
            auto newSys = std::make_unique<System<Args...>>(system, m_world);

            switch (schedule) {
                case Schedule::Startup:
                    m_startupSystems.emplace_back(std::move(newSys));
                break;
                case Schedule::Update:
                    LOG_CORE_TRACE("Added system");
                    m_updateSystems.emplace_back(std::move(newSys));
                break;
                case Schedule::FixedUpdate:
                    m_fixedUpdateSystems.emplace_back(std::move(newSys));
                break;
            }
        }

        void update(World& world) {
            for (const auto& system : m_updateSystems) {
                system->run(world);
            }
        }

    private:

        World& m_world;

        std::vector<std::unique_ptr<BaseSystem>> m_updateSystems;
        std::vector<std::unique_ptr<BaseSystem>> m_fixedUpdateSystems;
        std::vector<std::unique_ptr<BaseSystem>> m_startupSystems;


        // Deduces the parameter types (Args...) from the function pointer
        template <typename R, typename... Args>
        std::unique_ptr<BaseSystem> makeSystem(R(*func)(Args...), World& world) {
            // Creates and wraps your system, returning it as a generic BaseSystem*
            return std::make_unique<System<Args...>>(func, world);
        }

    };



}
