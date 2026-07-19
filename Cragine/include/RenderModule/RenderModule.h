#pragma once

#include "Core/App.h"
#include "Renderer.h"

namespace crg {

    class RenderModule : public Module {
        virtual void build(App& app) {
            app.addResource<renderer::RenderBackend>(app.getWindow());

            app.addSystem(Schedule::Startup, renderer::newMaterial);
            app.addSystem(Schedule::Update, renderer::render);
        }
    };

}
