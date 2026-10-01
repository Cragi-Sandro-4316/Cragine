#pragma once

#include <webgpu/webgpu.hpp>
#include "Ecs/Handle.h"

namespace crg::renderer {


    template<wgpu::ShaderStage Stage, typename ResourceType>
    struct MaterialParam {
        using Type = ResourceType;
        static constexpr wgpu::ShaderStage stage = Stage;
        Handle<ResourceType> handle;
    };

}
