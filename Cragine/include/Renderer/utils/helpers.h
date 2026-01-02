#pragma once

#include <webgpu.h>

namespace crg::renderer::helpers {
    WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options);
}
