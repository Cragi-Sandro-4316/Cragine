#include "App.h"
#include "Ecs/Entities/Entity.h"
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

        m_ecsWorld.spawnEntity<UnregisteredComponent>({
            UnregisteredComponent {
                .str = "LMAO XDDDD"
            }
        });

        m_ecsWorld.spawnEntity<UnregisteredComponent, int>({
            UnregisteredComponent {
                .str = "QUESTO ELEMENTP MUORE"
            },
            67
        });

        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            // LOG_CORE_TRACE("App currently running...");

            for (auto [ent, str] : m_ecsWorld.query<ecs::Entity, UnregisteredComponent>()) {
                LOG_CORE_TRACE("Entity of id: {}, gen {} says: {}", ent.index, ent.generation, str.str);
            }

            for (auto [ent, i] : m_ecsWorld.query<ecs::Entity, int>()) {
                m_ecsWorld.removeComponent<UnregisteredComponent>(ent);
            }

            for (auto [ent, str] : m_ecsWorld.query<ecs::Entity, UnregisteredComponent>()) {
                LOG_CORE_TRACE("Entity of id: {}, gen {} says: {}", ent.index, ent.generation, str.str);
            }

            break;
        }
        LOG_CORE_INFO("App terminated successfully");
    }
}
