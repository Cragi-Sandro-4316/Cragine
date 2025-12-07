#include "App.h"
#include "Ecs/Components/ComponentSignature.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Systems/SystemScheduler.h"
#include "Events/EventParam.h"
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

    struct Marker1 {
        std::string str;
    };
    struct Marker2 {};
    struct Marker3 {};


    void sampleSystem(
        ecs::EventWriter<Sample> writer
    ) {
        LOG_CORE_TRACE("System 1");
        writer.write(Sample {
            .str = "Porca troia funge"
        });

        writer.write(Sample {
            .str = "Evento 2"
        });

    }


    void sampleSystem2(
        ecs::EventReader<Sample> reader
    ) {
        LOG_CORE_TRACE("Sample Events");
        for (auto& [sample] : *reader.read()) {
            LOG_CORE_TRACE("Evento: {}", sample);
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
        m_systemScheduler.addSystem(ecs::Schedule::Update, sampleSystem2);

        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            // LOG_CORE_TRACE("App currently running...");


            m_systemScheduler.update(m_ecsWorld);

            break;
        }
        LOG_CORE_INFO("App terminated successfully");
    }







}
