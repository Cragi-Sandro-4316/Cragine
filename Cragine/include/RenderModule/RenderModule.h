#pragma once

#include "Core/App.h"
#include "Ecs/Ecs.h"
#include "RenderModule/Managers/BindGroupManager.h"
#include "RenderModule/Managers/MaterialManager.h"
#include "RenderModule/GpuInterface.h"
#include "RenderModule/RenderSystems.h"

namespace crg {

    class RenderModule : public Module {
        virtual void build(App& app) {
            app.addResource<renderer::GpuInterface>(app.getWindow());
            app.addResource<renderer::MaterialManager>();
            app.addResource<renderer::BindGroupManager>();

            app.addSystem(Startup, renderer::spawnExample);
            app.addSystem(ecs::Schedule::Update, renderer::render);
        }
    };

}
