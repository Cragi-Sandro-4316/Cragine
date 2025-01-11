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



    void Application::onEvent(Event& e) {

        EventDispatcher dispatcher(e);

        dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::onWindowClosed));

        LOG_TRACE("{0}", e.toString());
    }


    // the game update loop
    void Application::run() {

        // test event
        MouseButtonPressedEvent event(12);
        LOG_TRACE(event.toString());

        while (running) {
            window->onUpdate();
        }

    }

    bool Application::onWindowClosed(WindowCloseEvent& e) {
        running = false;

        return true;
    } 


}
