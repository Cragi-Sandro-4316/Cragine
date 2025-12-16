#include "App.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Systems/SystemScheduler.h"
#include "Events/EventParam.h"
#include "InputModule/InputManager.h"
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
        ecs::Res<InputManager> res
    ) {
        auto& inputManager = res.get();


        if (inputManager.keyPressed(KeyCode::KeyA)) {
            // LOG_CORE_INFO("You just pressed the A key!");
        }
        else {
            // LOG_CORE_FATAL("You did not press the A key!");
        }

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

        // m_ecsWorld.getResourceManager().newResource(Sample{
        //     .str = "pipaa"
        // });

        m_systemScheduler.addSystem(ecs::Schedule::Update, sampleSystem);

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
