#include "App.h"
#include "Ecs/Ecs.h"
#include "Ecs/Schedule.h"
#include "Ecs/SystemParams/ResParam.h"
#include "InputModule/InputManager.h"
#include "InputModule/KeyCode.h"
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


    struct Sample {
        std::string x = "sample";
    };

    struct Sample2 {
        std::string x = "sample2";
    };

    struct Foo {
        int x;
    };


    void inputTest(
        Res<InputManager> rInputManager
    ) {
        auto& inputManager = rInputManager.get();
        if (inputManager.keyPressed(KeyCode::KeyA)) {
            LOG_CORE_INFO("Pressed A");
        }

    }


    void App::run() {
        LOG_CORE_TRACE("App running");

        int i = 0;

        m_world.addSystem(Schedule::Update, inputTest);

        m_world.runSystems(Schedule::Startup);
        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();

            m_world.runCommands();

            m_world.runSystems(Schedule::Update);

            m_world.getEventManager()->swapBuffers();
            m_world.getEventManager()->clearAll();

            // i++;
            if (i == 3) {
                break;
            }
        }
        LOG_CORE_INFO("App terminated successfully");
    }







}
