#pragma once

#include <webgpu/webgpu.hpp>

namespace crg::renderer::helpers {
    wgpu::Adapter requestAdapterSync(wgpu::Instance instance, wgpu::RequestAdapterOptions const * options);

    /**
     * Utility function to get a WebGPU device, so that
     *     WGPUDevice device = requestDeviceSync(adapter, options);
     * is roughly equivalent to
     *     const device = await adapter.requestDevice(descriptor);
     * It is very similar to requestAdapter
     */
    wgpu::Device requestDeviceSync(wgpu::Adapter adapter, wgpu::DeviceDescriptor const * descriptor);

    /**
     * Round up "value" to the next multiple of step
     */
    uint32_t ceilToNextMultiple(
        uint32_t value,
        uint32_t step
    );
}
