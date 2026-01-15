#include "App.h"
#include "Ecs/Systems/SystemScheduler.h"
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

        LOG_CORE_TRACE("App running");

        int i = 0;
        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            // LOG_CORE_TRACE("Frame: {}", i);

            m_systemScheduler.update(m_ecsWorld);

            m_ecsWorld.getEventManager()->swapBuffers();
            // break;
            // i++;
            if (i == 1) {
                break;
            }
        }
        LOG_CORE_INFO("App terminated successfully");
    }







}
