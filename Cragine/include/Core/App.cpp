#include "App.h"
#include "Ecs/Entities/EntityManager.h"
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

        m_ecsWorld.spawnEntity<int>({
            67
        });

        m_ecsWorld.spawnEntity<UnregisteredComponent>({
            UnregisteredComponent {
                .str = "stocazzw"
            }
        });

        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            // LOG_CORE_TRACE("App currently running...");

            for (auto [ent] : m_ecsWorld.query<ecs::Entity>()) {
                LOG_CORE_INFO("Entity id: {}; Entity generation: {}", ent.index, ent.generation);
            }

            for (auto [ent, i] : m_ecsWorld.query<ecs::Entity, int>()) {
                m_ecsWorld.removeEntity(ent);
            }

        }
        LOG_CORE_INFO("App terminated successfully");
    }
}
