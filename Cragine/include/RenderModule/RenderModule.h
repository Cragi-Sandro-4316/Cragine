#pragma once

#include "Core/App.h"
#include "Ecs/Ecs.h"
#include "Ecs/Schedule.h"
#include "RenderModule/Managers/GpuResource.h"
#include "RenderModule/Managers/MaterialManager.h"
#include "RenderModule/GpuInterface.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/RenderSystems.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/ImageTexture.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/TextureAtlas.h"

namespace crg {

    class RenderModule : public Module {
        virtual void build(App& app) {
            app.addResource<renderer::RenderContext>(app.getWindow());
            app.addResource<renderer::MaterialManager>();
            app.addResource<renderer::GpuResource<renderer::IBuffer>>();
            app.addResource<renderer::GpuResource<renderer::ImageTexture>>();
            app.addResource<renderer::GpuResource<renderer::Sampler>>();
            app.addResource<renderer::GpuResource<renderer::TextureAtlas>>();


            app.addSystem(ecs::Schedule::Startup, renderer::startup);
            app.addSystem(ecs::Schedule::Update, renderer::spawnMeshes);
            app.addSystem(ecs::Schedule::Update, renderer::despawnMeshes);
            app.addSystem(ecs::Schedule::Update, renderer::render);
        }
    };

}
