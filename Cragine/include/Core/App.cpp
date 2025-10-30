#include "App.h"
#include "utils/Logger.h"

#include <GLFW/glfw3.h>
#include <string>

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

    struct UnregisteredComponent {
        std::string str;
    };

    void App::run() {

        m_ecsWorld.spawnEntity<UnregisteredComponent, int>({
            UnregisteredComponent {
                .str = "Gigio"
            },
            67
        });

        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            LOG_CORE_TRACE("App currently running...");
            LOG_CORE_ERROR("");

            for (auto [uc, in] : m_ecsWorld.query<UnregisteredComponent, int>()) {
                LOG_CORE_INFO("MEGAKNIGHT: {}", in);
            }

        }
        LOG_CORE_INFO("App terminated successfully");
    }
}
