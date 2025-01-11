#pragma once

#include "CraginePCH.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

#include "window/Window.h"

namespace Cragine {
    class Application {
    public:
        Application();
        virtual ~Application();

        void run();

        void onEvent(Event& e);

    private: 
        bool onWindowClosed(WindowCloseEvent& e);

        std::unique_ptr<Window> window;

        bool running = true;
    };


}

