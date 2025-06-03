#include "Application.h"
#include "utils/Logger.h"

#include "Events/ApplicationEvent.h"

#include "CraginePCH.h"

namespace Cragine {
    Application::Application() {}

    Application::~Application() {}

    void Application::run() {

        while(true) {
            WindowResizeEvent e(1280, 720);

            LOG_INFO(e.ToString());
        }
    }
}