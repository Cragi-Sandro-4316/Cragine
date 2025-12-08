#include "App.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Systems/SystemScheduler.h"
#include "Events/EventParam.h"
#include "Resources/ResourceParam.h"
#include "utils/Logger.h"
#include "Ecs/Components/QueryParam.h"

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
        ecs::ResMut<Sample> res
    ) {
        res.get().str = "gigio boos";
    }


    void sys2(
        ecs::Res<Sample> res
    ) {
        LOG_CORE_TRACE("{}", res.get().str);
    }

    void App::run() {

        m_ecsWorld.spawnEntity<Sample, Marker1>({
            Sample {
                .str = "Prova 1"
            },
            Marker1 {
            }
        });

        m_ecsWorld.spawnEntity<Sample, Marker2>({
            Sample {
                .str = "Prova 2"
            },
            Marker2 {}
        });

        m_ecsWorld.spawnEntity<Sample, Marker3>({
            Sample {
                .str = "Prova 3"
            },
            Marker3 {}
        });

        m_ecsWorld.getResourceManager().newResource(Sample{
            .str = "pipaa"
        });

        m_systemScheduler.addSystem(ecs::Schedule::Update, sampleSystem);
        m_systemScheduler.addSystem(ecs::Schedule::Update, sys2);

        int i = 0;
        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            LOG_CORE_TRACE("Frame: {}", i);

            m_systemScheduler.update(m_ecsWorld);

            m_ecsWorld.getEventManager()->swapBuffers();
            // break;
            i++;
            if (i == 1) {
                break;
            }
        }
        LOG_CORE_INFO("App terminated successfully");
    }







}
