#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Window/Window.h"

namespace Cragine {
    class Application {
    public:
        Application();
        virtual ~Application();

        void run();

    private:
        std::unique_ptr<Window> m_window;

        bool m_running = true; 
    };

    /// @brief To be defined in the client
    Application* createApplication();
}