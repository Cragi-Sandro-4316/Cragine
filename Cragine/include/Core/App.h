#pragma once
#include "Ecs/Systems/SystemScheduler.h"
#include "Ecs/World.h"
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
    private:

        ecs::World m_ecsWorld;

        ecs::SystemScheduler m_systemScheduler{m_ecsWorld};

        std::unique_ptr<Window> m_window;



    };
}
