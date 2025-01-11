#pragma once

#include "CraginePCH.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "layers/LayerStack.h"
#include "window/Window.h"

namespace Cragine {
    class Application {
    public:
        Application();
        virtual ~Application();

        void run();

        void onEvent(Event& e);

        void pushLayer(Layer* layer);
        void pushOverlay(Layer* layer);

    private: 
        bool onWindowClosed(WindowCloseEvent& e);

        std::unique_ptr<Window> window;

        bool running = true;

        LayerStack layerStack;
    };


}

