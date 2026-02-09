#include "App.h"
#include "Commands/Commands.h"
#include "Ecs/Components/QueryParam.h"
#include "Ecs/Components/QueryResult.h"
#include "Ecs/Entities/Entity.h"
#include "Ecs/Systems/SystemScheduler.h"
#include "InputModule/InputManager.h"
#include "Rendererr/Components/MeshHandle.h"
#include "Resources/ResourceParam.h"
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

    void spawnCam (
        ecs::Res<InputManager> inputManager,
        ecs::Commands commands
    ) {

        LOG_CORE_INFO("Mouse position: x {}, y {}",
            inputManager.get().mousePosition().first,
            inputManager.get().mousePosition().second
        );
        commands.get().spawnEntity<MeshHandle>({
            MeshHandle{
                .path = "/pyramid.obj"
            }
        });

    }

    void App::run() {

        LOG_CORE_TRACE("App running");

        addSystem(ecs::Schedule::Startup, spawnCam);


        int i = 0;
        m_systemScheduler.startup(m_ecsWorld);
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
