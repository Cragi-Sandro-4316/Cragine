#include "App.h"
#include "Ecs/Components/ComponentSignature.h"
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


    struct Sample {
        std::string str;
    };

    struct Marker1 {};

    struct Marker2 {};

    void App::run() {

        m_ecsWorld.spawnEntity<Sample, Marker1>({
            Sample {
                .str = "Pipa"
            },
            Marker1 {
            }
        });

        m_ecsWorld.spawnEntity<Sample, Marker2>({
            Sample {
                .str = "Maurizio"
            },
            Marker2 {}
        });

        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            // LOG_CORE_TRACE("App currently running...");

            for (
                auto [str] :
                m_ecsWorld.query<
                    Sample,
                    ecs::With<Marker2>
                >()
            ) {
                LOG_CORE_ERROR("Entity says: {}", str.str);

            }

            for (
                auto [entity, str] :
                m_ecsWorld.query<
                    ecs::Entity,
                    Sample,
                    ecs::With<Marker1>,
                    ecs::Without<Marker2>
                >()
            ) {
                LOG_CORE_ERROR("Entity says: {}", str.str);

                m_ecsWorld.addComponent(entity, Marker2{});
            }

            for (
                auto [str] :
                m_ecsWorld.query<
                    Sample,
                    ecs::With<Marker2>
                >()
            ) {
                LOG_CORE_ERROR("Entity says: {}", str.str);

            }

            break;
        }
        LOG_CORE_INFO("App terminated successfully");
    }
}
