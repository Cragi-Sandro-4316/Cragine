#include "App.h"
#include "Ecs/Components/ComponentSignature.h"
#include "Ecs/Components/QueryResult.h"
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
    struct Marker3 {};


    void sampleSystem(ecs::Query<Sample, ecs::Without<Marker1>> query) {
        for (auto [sample] : query.iter()) {
            LOG_CORE_TRACE("{}", sample.str);
        }
    }

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

        m_ecsWorld.spawnEntity<Sample, Marker3>({
            Sample {
                .str = "Romolo e Remo"
            },
            Marker3 {}
        });

        m_systemScheduler.addSystem(ecs::Schedule::Update, sampleSystem);

        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            // LOG_CORE_TRACE("App currently running...");


            m_systemScheduler.update(m_ecsWorld);

            break;
        }
        LOG_CORE_INFO("App terminated successfully");
    }







}
