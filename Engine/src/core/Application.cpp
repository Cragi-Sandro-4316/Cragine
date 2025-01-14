#include "core/Application.h"

#include "Events/MouseEvent.h"
#include "utils/Logger.h"
#include "window/Window.h"

namespace Cragine {


    #define BIND_EVENT_FN(func) std::bind(&func, this, std::placeholders::_1)

    Application::Application() {
        window = std::unique_ptr<Window>(Window::create());
        window->setEventCallback(BIND_EVENT_FN(Application::onEvent));
    }

    Application::~Application() {}


    void Application::pushLayer(Layer* layer) {
        layerStack.pushLayer(layer);
    }

    void Application::pushOverlay(Layer* layer) {
        layerStack.pushOverlay(layer);
    }


    void Application::onEvent(Event& e) {

        EventDispatcher dispatcher(e);

        dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::onWindowClosed));


        // Iterates the layers backwards and handles the event
        for (auto it = layerStack.end(); it != layerStack.begin();) {
            (*--it)->onEvent(e);
            if (e.handled) 
                break;
        }
    }


    // the game update loop
    void Application::run() {

        while (running) {
            window->onUpdate();

            // Updates the layers
            for (Layer* layer : layerStack) {
                layer->onUpdate();
            }
        }

    }

    bool Application::onWindowClosed(WindowCloseEvent& e) {
        running = false;

        return true;
    } 


}
