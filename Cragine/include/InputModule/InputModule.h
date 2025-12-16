#pragma once

#include "Ecs/Systems/SystemScheduler.h"
#include "InputManager.h"
#include "Module/Module.h"
#include "Core/App.h"
#include "InputManager.h"
#include "Resources/ResourceParam.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>

namespace crg {


    class InputModule : public Module {
        virtual void build(App& app) {

            app.addResource<InputManager>(app.getWindow());
            app.addSystem(ecs::Schedule::Update, update);
        }

        static void update(ecs::ResMut<InputManager> res) {
            auto inputManager = res.get();
            inputManager.update();

            // LOG_CORE_TRACE("Updated input manager");
        }
    };




}
