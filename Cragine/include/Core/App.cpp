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

    struct Component {
        std::string str;
    };

    void App::run() {
        m_ecsWorld.registerComponent<Component>();
        m_ecsWorld.registerComponent<int>();

        m_ecsWorld.spawnEntity<Component>({
            Component{
                .str = "Stocazzw"
            }
        });

        m_ecsWorld.spawnEntity<Component, int>({
            Component{
                .str = "Roberto"
            },
            9
        });

        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            LOG_CORE_TRACE("App currently running...");
            LOG_CORE_ERROR("");


            for (auto [component] : m_ecsWorld.query<Component>()) {
                LOG_CORE_INFO("ENTITY COMPONENT VALUE: {}", component.str);
            }

            for (auto [component, integer] : m_ecsWorld.query<Component, int>()) {
                LOG_CORE_INFO("ENTITY COMPONENT VALUE: {} {}", component.str, integer);
                component.str = "Jovanotti";
            }
        }
        LOG_CORE_INFO("App terminated successfully");
    }
}
