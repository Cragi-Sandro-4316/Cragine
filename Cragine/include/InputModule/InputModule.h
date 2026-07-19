#pragma once

#include "InputManager.h"
#include "Module/Module.h"
#include "Core/App.h"
#include "InputManager.h"
#include <GLFW/glfw3.h>

namespace crg {


    class InputModule : public Module {
        virtual void build(App& app) {

            app.addResource<InputManager>(app.getWindow()->getGlfwWindow());
            app.addSystem(ecs::Schedule::Update, updateInputs);
        }

        static void updateInputs(
            ResMut<InputManager> rInputManager
        ) {
            auto& inputManager = rInputManager.get();
            inputManager.update();

        }
    };




}
