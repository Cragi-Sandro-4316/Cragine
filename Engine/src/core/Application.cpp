#include "core/Application.h"

#include "Events/MouseEvent.h"
#include "utils/Logger.h"
#include "window/Window.h"
// #include "input/Input.h"
#include "glad/glad.h"

namespace Cragine {

    #define BIND_EVENT_FN(func) std::bind(&func, this, std::placeholders::_1)

    Application* Application::instance = nullptr;

    Application::Application() {
        instance = this;
        
        window = std::unique_ptr<Window>(Window::create());
        window->setEventCallback(BIND_EVENT_FN(Application::onEvent));

        imGuiLayer = new ImGuiLayer;
        pushOverlay(imGuiLayer);
    }

    Application::~Application() {}


    void Application::pushLayer(Layer* layer) {
        layerStack.pushLayer(layer);
        layer->onAttach();
    }

    void Application::pushOverlay(Layer* layer) {
        layerStack.pushOverlay(layer);
        layer->onAttach();
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
            glClearColor(0.3, 0, 0.3, 1);
            glClear(GL_COLOR_BUFFER_BIT);


            // Updates the layers
            for (Layer* layer : layerStack) {
                layer->onUpdate();
            }

            imGuiLayer->begin();
            for (Layer* layer : layerStack) {
                layer->onImGuiRender();
            }
            imGuiLayer->end();

            window->onUpdate();
        }

    }

    bool Application::onWindowClosed(WindowCloseEvent& e) {
        running = false;

        return true;
    } 


}
