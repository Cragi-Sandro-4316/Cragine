#pragma once

#include "RenderModule/GpuResourceManager.h"
#include "RenderModule/Structs/Material.h"

namespace crg::renderer {

    struct MaterialUpdate {

        using FuncType = void (*)(Material& material, GpuResourceManager& resManager);

        Handle<Material> material;

        FuncType update;
    };


}
