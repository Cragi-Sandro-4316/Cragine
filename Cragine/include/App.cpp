#include "App.h"
#include "utils/Logger.h"

#include <GLFW/glfw3.h>

namespace crg {

    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    App::App() {
        Logger::init();

        m_window = std::make_unique<Window>(
            WIDTH,
            HEIGHT,
            "Pipa Engine"
        );
    }

    void App::run() {
        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            LOG_CORE_TRACE("App currently running...");
        }
        LOG_CORE_INFO("Uscito con successo");
    }
}
