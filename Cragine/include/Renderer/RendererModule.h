#pragma once

#include "Module/Module.h"
#include "Core/App.h"
#include "Renderer/Renderer.h"

namespace crg {

    class RendererModule : public Module {
        virtual void build(App& app) {
            app.addResource<renderer::Renderer>(app.getWindow()->getGlfwWindow());


        }

    };

}
