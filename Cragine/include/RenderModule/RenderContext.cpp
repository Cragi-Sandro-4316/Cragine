#include "Window.h"
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu.h>
#include <webgpu/webgpu.hpp>
#include <wgpu.h>

#include "RenderContext.h"
#include "glfw3webgpu.h"
#include "utils/Logger.h"

namespace crg::renderer {

    RenderContext::RenderContext(Window* window)
    : window(window) {
        if (!initInstance()) return;

        if (!initSurface()) return;

        getAdapter();

        initDevice();
        queue = device.getQueue();

        surface.getCapabilities(adapter, &capabilities);

        surfaceFormat = capabilities.formats[0];

        config = wgpu::SurfaceConfiguration{};
        config.format = capabilities.formats[0];
        config.nextInChain = nullptr;
        config.width = window->getWidth();
        config.height = window->getHeight();

        config.viewFormatCount = 0;
        config.viewFormats = nullptr;

        config.usage = WGPUTextureUsage_RenderAttachment;
        config.device = device;
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Auto;

        surface.configure(config);

        LOG_CORE_INFO("Created gpu handler");
    }


    bool RenderContext::initInstance() {
        wgpu::InstanceDescriptor desc{};
        desc.nextInChain = nullptr;

        instance = wgpu::createInstance(desc);

        if (!instance) {
            LOG_CORE_ERROR("Could not initialize wgpu");
            return false;
        }
        return true;
    }

    bool RenderContext::initSurface() {
        surface = glfwCreateWindowWGPUSurface(instance, window->getGlfwWindow());

       if (!surface) {
           LOG_CORE_ERROR("Could not initialize Surface");
           return false;
       }
       return true;
    }

    void RenderContext::getAdapter() {
        wgpu::RequestAdapterOptions adaptOpt{};
        adaptOpt.nextInChain = nullptr;
        adaptOpt.compatibleSurface = surface;

        adapter = instance.requestAdapter(adaptOpt);
    }

    wgpu::Limits RenderContext::getRequiredLimits() {
        wgpu::Limits supportedLimits;
        adapter.getLimits(&supportedLimits);

        return supportedLimits;
    }

    void RenderContext::initDevice() {
        auto limits = getRequiredLimits();

        wgpu::SupportedFeatures supportedFeatures;
        adapter.getFeatures(&supportedFeatures);

        // TODO: Cleanup feature requesting
        LOG_CORE_INFO("vertex writable storage: {}", (int)WGPUNativeFeature_VertexWritableStorage);

        LOG_CORE_INFO("Supported features: ");
        for (size_t i = 0;  i < supportedFeatures.featureCount; i++) {
            LOG_CORE_INFO("{}", (int)supportedFeatures.features[i]);
        }

        wgpu::DeviceDescriptor deviceDescriptor{};

        deviceDescriptor.label = WGPUStringView("Device");
        deviceDescriptor.nextInChain = nullptr;
        deviceDescriptor.requiredFeatureCount = supportedFeatures.featureCount;
        deviceDescriptor.requiredFeatures = supportedFeatures.features;
        deviceDescriptor.requiredLimits = &limits;
        deviceDescriptor.defaultQueue.label = WGPUStringView("Default queue");
        deviceDescriptor.defaultQueue.nextInChain = nullptr;


        auto lostCallback = [](WGPUDeviceImpl* const *, WGPUDeviceLostReason reason, WGPUStringView message, void*, void*) {
            LOG_CORE_ERROR("Device lost: reason {}", (char*)reason);
            if (message.data) {
                LOG_CORE_ERROR("{}", message.data);
            }
        };

        wgpu::DeviceLostCallbackInfo callbackInfo{};
        callbackInfo.nextInChain = nullptr;
        callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
        callbackInfo.callback = lostCallback;

        deviceDescriptor.deviceLostCallbackInfo = callbackInfo;

        auto errorCallback = [](WGPUDeviceImpl *const *, WGPUErrorType err, WGPUStringView msg, void *, void *){
            LOG_CORE_ERROR("Device lost: reason {}", (int)err);
            if (msg.data) {
                LOG_CORE_ERROR("{}", msg.data);
            }
        };
        wgpu::UncapturedErrorCallbackInfo errorCallbackInfo{};
        errorCallbackInfo.nextInChain = nullptr;
        errorCallbackInfo.callback = errorCallback;


        device = adapter.requestDevice(deviceDescriptor);
    }

}
