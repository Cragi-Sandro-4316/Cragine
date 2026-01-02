#include "Renderer.h"
#include "Renderer/utils/helpers.h"
#include "utils/Logger.h"
#include <webgpu.h>

namespace crg::renderer {
    Renderer::Renderer() {
        LOG_CORE_INFO("Initializing renderer");

        // Gets the instance
        WGPUInstanceDescriptor desc{};
        desc.nextInChain = nullptr;

        m_instance = wgpuCreateInstance(&desc);
        if (!m_instance) {
            LOG_CORE_ERROR("Could not initialize wgpu instance!");
            return;
        }

        // Gets the gpu adapter
        LOG_CORE_INFO("Requesting adapter...");
        WGPURequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        WGPUAdapter adapter = helpers::requestAdapterSync(m_instance, &adapterOptions);

        LOG_CORE_INFO("Got adapter: {}", (void*)adapter);

        // Prints gpu limits
        WGPULimits supportedLimits{};
        supportedLimits.nextInChain = nullptr;

        bool success = wgpuAdapterGetLimits(adapter, &supportedLimits);

        if (success) {
            LOG_CORE_INFO("Adapter limits: ");
            LOG_CORE_INFO(" - maxTextureDimension1D: {}", supportedLimits.maxTextureDimension1D);
            LOG_CORE_INFO(" - maxTextureDimension2D: {}", supportedLimits.maxTextureDimension2D);
            LOG_CORE_INFO(" - maxTextureDimension3D: {}", supportedLimits.maxTextureDimension3D);
            LOG_CORE_INFO(" - maxTextureArrayLayers: {}", supportedLimits.maxTextureArrayLayers);
        }

        WGPUSupportedFeatures supportedFeatures{};
        wgpuAdapterGetFeatures(adapter, &supportedFeatures);
        LOG_CORE_INFO("Supported features: ");
        for (size_t i = 0; i < supportedFeatures.featureCount; i++) {
            WGPUFeatureName feature = supportedFeatures.features[i];
            LOG_CORE_INFO("Adapter feature: {}", static_cast<int>(feature));
        }

        WGPUAdapterInfo info{};
        wgpuAdapterGetInfo(adapter, &info);

        LOG_CORE_INFO("Adapter info: ");
        if (info.vendor.data) {
            LOG_CORE_INFO("Vendor: {}", std::string(info.vendor.data, info.vendor.length));
        }

        if (info.architecture.data) {
            LOG_CORE_INFO("Architecture: {}", std::string(info.architecture.data, info.architecture.length));
        }

        if (info.device.data) {
            LOG_CORE_INFO("Device: {}", std::string(info.device.data, info.device.length));
        }

        if (info.description.data) {
            LOG_CORE_INFO("Description: {}", std::string(info.description.data, info.description.length));
        }

        LOG_CORE_INFO("Adapter type: {}", static_cast<int>(info.adapterType));
        LOG_CORE_INFO("Backend type: {}", static_cast<int>(info.backendType));

    }

    Renderer::~Renderer() {
        wgpuInstanceRelease(m_instance);
        LOG_CORE_INFO("Released wgpu instance");
    }

}
