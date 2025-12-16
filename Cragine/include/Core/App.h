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

        App& addModule(std::vector<Module>& modules) {
            for (auto& module : modules ) {
                module.build(*this);
            }

            return *this;
        }

        template <typename R, typename... Args>
        void addSystem(const ecs::Schedule schedule, R (*system)(Args...)) {
            m_systemScheduler.addSystem(schedule, system);
        }

        template<typename ResourceName, typename... Args>
        void addResource(Args&&... args) {
            m_ecsWorld.getResourceManager().newResource<ResourceName>(
                std::forward<Args>(args)...
            );
        }

        Window* getWindow() {
            return m_window.get();
        }

    private:

        ecs::World m_ecsWorld;

        ecs::SystemScheduler m_systemScheduler{m_ecsWorld};

        std::unique_ptr<Window> m_window;

    };
}
