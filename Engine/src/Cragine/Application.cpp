#include "Application.h"
#include "utils/Logger.h"

#include "Events/ApplicationEvent.h"

#include "CraginePCH.h"

namespace Cragine {
    Application::Application() {
        m_window = std::unique_ptr<Window>(Window::Create());
    }

    Application::~Application() {}

    void Application::run() {

        while(m_running) {
            m_window->onUpdate();
        }
    }
}