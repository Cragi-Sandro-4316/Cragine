#pragma once

#include "AssetManager/AssetManagerModule.h"
#include "InputModule/InputModule.h"
#include "Module/Module.h"
#include "Core/App.h"
#include "RenderModule/RenderModule.h"

namespace crg {
    class DefaultModules : public Module {

        virtual void build(App& app) {
            app.addModule(InputModule{});
            app.addModule(AssetManagerModule{});
            app.addModule(RenderModule{});

        }

    };
}
