#pragma once
#include "Ecs/Systems/SystemScheduler.h"
#include "Ecs/World.h"
#include "Module/Module.h"
#include "Window.h"
#include <memory>


namespace crg {
    class App {
    public:
        App();
        // ~App();

        App (const App&) = delete;
        App& operator= (const App&) = delete;

        void run();

        App& addModule(Module&& module) {
            module.build(*this);


            return *this;
        }

        template <typename R, typename... Args>
        void addSystems(const ecs::Schedule schedule, R (*system)(Args...)) {
            m_systemScheduler.addSystem(schedule, system);
        }

    private:

        ecs::World m_ecsWorld;

        ecs::SystemScheduler m_systemScheduler{m_ecsWorld};

        std::unique_ptr<Window> m_window;

    };
}
