#pragma once

#include "Core/App.h"
#include "Ecs/Systems/SystemScheduler.h"
#include "Module/Module.h"
#include "Rendererr/Components/MeshPool.h"
#include "Rendererr/Components/ShaderManager.h"
#include "Rendererr/RenderContext.h"
#include "Rendererr/FrameData.h"
#include "Ecs/Components/QueryResult.h"

namespace crg {
    class RendererModule : public Module {
        virtual void build(App& app) {
            app.addResource<renderer::ShaderManager>();
            app.addResource<renderer::RenderContext>(app.getWindow());
            app.addResource<renderer::FrameData>();
            app.addResource<renderer::MeshPool>();

            app.addSystem(ecs::Startup, renderer::initPipeline);

            app.addSystem(ecs::Update, renderer::extractData);
            app.addSystem(ecs::Update, renderer::loadMeshes);
            app.addSystem(ecs::Update, renderer::drawFrame);
        }


    };
}
