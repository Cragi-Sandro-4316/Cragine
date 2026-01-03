#include "Renderer.h"
#include "Renderer/utils/helpers.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <webgpu.h>
#include <wgpu.h>
#include <glfw3webgpu.h>

namespace crg::renderer {
    Renderer::Renderer(Window* window) :
    m_window(window) {
        LOG_CORE_INFO("Initializing renderer");

        fetchInstance();

        getSurface();

        fetchAdapter();

        printAdapterInfo();

        fetchDevice();

        configureSurface();


        fetchQueue();
    }

    Renderer::~Renderer() {
        wgpuQueueRelease(m_queue);
        LOG_CORE_INFO("Released wgpu queue");

        wgpuDeviceRelease(m_device);
        wgpuAdapterRelease(m_adapter);
        LOG_CORE_INFO("Released wgpu device and adapter");

        wgpuSurfaceUnconfigure(m_surface);
        wgpuSurfaceRelease(m_surface);
        LOG_CORE_INFO("Released wgpu surface");

        wgpuInstanceRelease(m_instance);
        LOG_CORE_INFO("Released wgpu instance");
    }



    void Renderer::fetchInstance() {
        // Gets the instance
        WGPUInstanceDescriptor desc{};
        desc.nextInChain = nullptr;

        m_instance = wgpuCreateInstance(&desc);
        if (!m_instance) {
            LOG_CORE_ERROR("Could not initialize wgpu instance!");
            return;
        }
    }

    void Renderer::getSurface() {
        m_surface = glfwCreateWindowWGPUSurface(m_instance, m_window->getGlfwWindow());
        if (!m_surface) {
            LOG_CORE_ERROR("Could not initialize wgpu surface!");

            int platform = glfwGetPlatform();
            LOG_CORE_INFO("GLFW platform: {}", platform);

            if (platform == GLFW_PLATFORM_WAYLAND) {
                LOG_CORE_ERROR("Running on wayland");
            }
            else {
                LOG_CORE_ERROR("Running on x11");
            }

            return;
        }
    }

    void Renderer::configureSurface() {
        WGPUSurfaceConfiguration config{};
        config.nextInChain = nullptr;
        config.width = m_window->getWidth();
        config.height = m_window->getHeight();

        WGPUSurfaceCapabilities capabilities;
        wgpuSurfaceGetCapabilities(m_surface, m_adapter, &capabilities);

        config.format = capabilities.formats[0];

        config.viewFormatCount = 0;
        config.viewFormats = nullptr;

        config.usage = WGPUTextureUsage_RenderAttachment;
        config.device = m_device;
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Auto;

        wgpuSurfaceConfigure(m_surface, &config);
    }


    void Renderer::fetchAdapter() {
        // Gets the gpu adapter
        LOG_CORE_INFO("Requesting adapter...");
        WGPURequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        adapterOptions.compatibleSurface = m_surface;
        m_adapter = helpers::requestAdapterSync(m_instance, &adapterOptions);

        LOG_CORE_INFO("Got adapter: {}", (void*)m_adapter);
    }

    void Renderer::printAdapterInfo() {
        // Prints gpu limits
        WGPULimits supportedLimits{};
        supportedLimits.nextInChain = nullptr;

        bool success = wgpuAdapterGetLimits(m_adapter, &supportedLimits);

        if (success) {
            LOG_CORE_INFO("Adapter limits: ");
            LOG_CORE_INFO(" - maxTextureDimension1D: {}", supportedLimits.maxTextureDimension1D);
            LOG_CORE_INFO(" - maxTextureDimension2D: {}", supportedLimits.maxTextureDimension2D);
            LOG_CORE_INFO(" - maxTextureDimension3D: {}", supportedLimits.maxTextureDimension3D);
            LOG_CORE_INFO(" - maxTextureArrayLayers: {}", supportedLimits.maxTextureArrayLayers);
        }

        WGPUSupportedFeatures supportedFeatures{};
        wgpuAdapterGetFeatures(m_adapter, &supportedFeatures);
        LOG_CORE_INFO("Supported features: ");
        for (size_t i = 0; i < supportedFeatures.featureCount; i++) {
            WGPUFeatureName feature = supportedFeatures.features[i];
            LOG_CORE_INFO("Adapter feature: {}", static_cast<int>(feature));
        }

        WGPUAdapterInfo info{};
        wgpuAdapterGetInfo(m_adapter, &info);

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

    void Renderer::fetchDevice() {
        LOG_CORE_INFO("Requesting device...");

        WGPUDeviceDescriptor deviceDescriptor{};
        deviceDescriptor.nextInChain = nullptr;
        deviceDescriptor.requiredFeatureCount = 0;
        deviceDescriptor.requiredLimits = nullptr;
        deviceDescriptor.defaultQueue.nextInChain = nullptr;
        deviceDescriptor.defaultQueue.label = WGPUStringView("The default queue");

        auto lostCallback = [](WGPUDeviceImpl * const *, WGPUDeviceLostReason reason, WGPUStringView message, void *, void *) {
            LOG_CORE_ERROR("Device lost: reason {}", (int)reason);
            if (message.data) {
                LOG_CORE_ERROR("{}", message.data);
            }
        };

        WGPUDeviceLostCallbackInfo callbackInfo{};
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
        WGPUUncapturedErrorCallbackInfo errorCallbackInfo{};

        errorCallbackInfo.nextInChain = nullptr;
        errorCallbackInfo.callback = errorCallback;


        m_device = helpers::requestDeviceSync(m_adapter, &deviceDescriptor);
        LOG_CORE_INFO("Got device: {}", (void*)m_device);
    }


    void Renderer::fetchQueue() {
        m_queue = wgpuDeviceGetQueue(m_device);

        auto onQueueWorkDoneCallback = [](WGPUQueueWorkDoneStatus status, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2){
            LOG_CORE_INFO("Queue work finished with status: {}", (int)status);
        };
        WGPUQueueWorkDoneCallbackInfo workCallbackInfo{};
        workCallbackInfo.callback = onQueueWorkDoneCallback;
        workCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;

        wgpuQueueOnSubmittedWorkDone(m_queue, workCallbackInfo);
    }


    std::pair<WGPUSurfaceTexture, WGPUTextureView> Renderer::getNextSurfaceViewData() {
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
            return { surfaceTexture, nullptr };
        }

        WGPUTextureViewDescriptor viewDescriptor{};
        viewDescriptor.nextInChain = nullptr;
        viewDescriptor.label = WGPUStringView("Surface texture view");
        viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
        viewDescriptor.dimension = WGPUTextureViewDimension_2D;
        viewDescriptor.baseMipLevel = 0;
        viewDescriptor.mipLevelCount = 1;
        viewDescriptor.baseArrayLayer = 0;
        viewDescriptor.arrayLayerCount = 1;
        viewDescriptor.aspect = WGPUTextureAspect_All;

        WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

        return { surfaceTexture, targetView };
    }


    void Renderer::update() {
        auto [ surfaceTexture, targetView ] = getNextSurfaceViewData();
        if (!targetView) return;

        WGPUCommandEncoderDescriptor encoderDesc{};
        encoderDesc.nextInChain = nullptr;
        encoderDesc.label = WGPUStringView("Frame encoder");

        auto encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);


        WGPURenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;

        WGPURenderPassColorAttachment renderPassColorAttachment{};

        renderPassColorAttachment.view = targetView;
        renderPassColorAttachment.resolveTarget = nullptr;

        renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
        renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
        renderPassColorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};
        renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.timestampWrites = nullptr;

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);


        WGPUCommandBufferDescriptor cmdDesc = {};
        WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, &cmdDesc);

        // Finally submit the command queue

        wgpuQueueSubmit(m_queue, 1, &cmd);
        wgpuCommandBufferRelease(cmd);
        wgpuCommandEncoderRelease(encoder); // release encoder after it's finished


        wgpuSurfacePresent(m_surface);
        wgpuTextureViewRelease(targetView);
        wgpuDevicePoll(m_device, false, nullptr);
    }

}
