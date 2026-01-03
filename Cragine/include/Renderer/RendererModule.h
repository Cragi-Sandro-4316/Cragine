#pragma once

#include "Module/Module.h"
#include "Core/App.h"
#include "Renderer/Renderer.h"
#include "Resources/ResourceParam.h"

namespace crg {

    class RendererModule : public Module {
        virtual void build(App& app) {
            app.addResource<renderer::Renderer>(app.getWindow());

            app.addSystem(ecs::Schedule::Update, updateRenderer);
        }

        static void updateRenderer(ecs::ResMut<renderer::Renderer> rendererRes) {
            auto& renderer = rendererRes.get();
            renderer.update();
        }

    };

}
