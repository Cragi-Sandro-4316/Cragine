#include "Renderer.h"
#include "utils/Logger.h"
#include <webgpu.h>

namespace crg::renderer {
    Renderer::Renderer() {
        LOG_CORE_INFO("Initializing renderer");

        WGPUInstanceDescriptor desc{};
        desc.nextInChain = nullptr;

        m_instance = wgpuCreateInstance(&desc);
        if (!m_instance) {
            LOG_CORE_ERROR("Could not initialize wgpu instance!");
            return;
        }

    }

    Renderer::~Renderer() {
        wgpuInstanceRelease(m_instance);
        LOG_CORE_INFO("Released wgpu instance");
    }

}
