#pragma once

#include "InputModule/InputModule.h"
#include "Module/Module.h"
#include "Core/App.h"
#include "Renderer/RendererModule.h"

namespace crg {
    class DefaultModules : public Module {

        virtual void build(App& app) {
            app.addModule(InputModule{});
            app.addModule(RendererModule{});
        }

    };
}
