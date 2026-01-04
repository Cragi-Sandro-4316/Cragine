#include <unistd.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "Renderer.h"
#include "Renderer/utils/helpers.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

namespace crg::renderer {

    const  char* shaderSource = R"(
        @vertex
        fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
           	var p = vec2f(0.0, 0.0);
           	if (in_vertex_index == 0u) {
          		p = vec2f(-0.5, -0.5);
           	} else if (in_vertex_index == 1u) {
          		p = vec2f(0.5, -0.5);
           	} else {
          		p = vec2f(0.0, 0.5);
           	}
           	return vec4f(p, 0.0, 1.0);
        }

        @fragment
        fn fs_main() -> @location(0) vec4f {
       	    return vec4f(0.0, 0.4, 1.0, 1.0);
        }


    )";


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

        makePipeline();
    }

    Renderer::~Renderer() {
        m_pipeline.release();
        LOG_CORE_INFO("Released wgpu pipeline");

        m_queue.release();
        LOG_CORE_INFO("Released wgpu queue");

        m_device.release();
        m_adapter.release();
        LOG_CORE_INFO("Released wgpu device and adapter");

        m_surface.unconfigure();
        m_surface.release();
        LOG_CORE_INFO("Released wgpu surface");

        m_instance.release();
        LOG_CORE_INFO("Released wgpu instance");
    }



    void Renderer::fetchInstance() {
        // Gets the instance
        wgpu::InstanceDescriptor desc{};
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
            return;
        }
    }

    void Renderer::configureSurface() {
        wgpu::SurfaceConfiguration config{};
        config.nextInChain = nullptr;
        config.width = m_window->getWidth();
        config.height = m_window->getHeight();

        wgpu::SurfaceCapabilities capabilities;
        m_surface.getCapabilities(m_adapter, &capabilities);

        m_surfaceFormat = capabilities.formats[0];
        config.format = capabilities.formats[0];

        config.viewFormatCount = 0;
        config.viewFormats = nullptr;

        config.usage = WGPUTextureUsage_RenderAttachment;
        config.device = m_device;
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Auto;

        m_surface.configure(config);
    }


    void Renderer::fetchAdapter() {
        // Gets the gpu adapter
        LOG_CORE_INFO("Requesting adapter...");
        wgpu::RequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        adapterOptions.compatibleSurface = m_surface;
        m_adapter = helpers::requestAdapterSync(m_instance, &adapterOptions);

        LOG_CORE_INFO("Got adapter: {}", (void*)m_adapter);
    }

    void Renderer::printAdapterInfo() {
        // Prints gpu limits
        wgpu::Limits supportedLimits{};
        supportedLimits.nextInChain = nullptr;

        bool success = m_adapter.getLimits(&supportedLimits);

        if (success) {
            LOG_CORE_INFO("Adapter limits: ");
            LOG_CORE_INFO(" - maxTextureDimension1D: {}", supportedLimits.maxTextureDimension1D);
            LOG_CORE_INFO(" - maxTextureDimension2D: {}", supportedLimits.maxTextureDimension2D);
            LOG_CORE_INFO(" - maxTextureDimension3D: {}", supportedLimits.maxTextureDimension3D);
            LOG_CORE_INFO(" - maxTextureArrayLayers: {}", supportedLimits.maxTextureArrayLayers);
        }

        wgpu::SupportedFeatures supportedFeatures{};
        m_adapter.getFeatures(&supportedFeatures);
        LOG_CORE_INFO("Supported features: ");
        for (size_t i = 0; i < supportedFeatures.featureCount; i++) {
            WGPUFeatureName feature = supportedFeatures.features[i];
            LOG_CORE_INFO("Adapter feature: {}", static_cast<int>(feature));
        }

        wgpu::AdapterInfo info{};
        m_adapter.getInfo(&info);

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

        wgpu::DeviceDescriptor deviceDescriptor{};
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


        m_device = helpers::requestDeviceSync(m_adapter, &deviceDescriptor);
        LOG_CORE_INFO("Got device: {}", (void*)m_device);
    }


    void Renderer::fetchQueue() {
        m_queue = m_device.getQueue();

        auto onQueueWorkDoneCallback = [](WGPUQueueWorkDoneStatus status, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2){
            LOG_CORE_INFO("Queue work finished with status: {}", (int)status);
        };
        wgpu::QueueWorkDoneCallbackInfo workCallbackInfo{};
        workCallbackInfo.callback = onQueueWorkDoneCallback;
        workCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;

        m_queue.onSubmittedWorkDone(workCallbackInfo);
    }


    std::pair<wgpu::SurfaceTexture, wgpu::TextureView> Renderer::getNextSurfaceViewData() {
        wgpu::SurfaceTexture surfaceTexture;
        m_surface.getCurrentTexture(&surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
            return { surfaceTexture, nullptr };
        }

        wgpu::TextureViewDescriptor viewDescriptor{};
        viewDescriptor.nextInChain = nullptr;
        viewDescriptor.label = WGPUStringView("Surface texture view");
        viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
        viewDescriptor.dimension = WGPUTextureViewDimension_2D;
        viewDescriptor.baseMipLevel = 0;
        viewDescriptor.mipLevelCount = 1;
        viewDescriptor.baseArrayLayer = 0;
        viewDescriptor.arrayLayerCount = 1;
        viewDescriptor.aspect = WGPUTextureAspect_All;

        wgpu::TextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

        return { surfaceTexture, targetView };
    }


    void Renderer::makePipeline() {
        wgpu::RenderPipelineDescriptor desc{};

        desc.vertex.bufferCount = 0;
        desc.vertex.buffers = nullptr;

        wgpu::ShaderModuleDescriptor shaderDesc{};

        wgpu::ShaderSourceWGSL shaderCodeDesc{};
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
        shaderCodeDesc.code = wgpu::StringView(shaderSource);

        shaderDesc.nextInChain = &shaderCodeDesc.chain;

        wgpu::ShaderModule shaderModule = m_device.createShaderModule(shaderDesc);

        desc.vertex.module = shaderModule;
        desc.vertex.entryPoint = wgpu::StringView("vs_main");
        desc.vertex.constantCount = 0;
        desc.vertex.constants = nullptr;

        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
        desc.primitive.frontFace = wgpu::FrontFace::CCW;
        desc.primitive.cullMode = wgpu::CullMode::None;

        wgpu::FragmentState fragState{};
        fragState.module = shaderModule;
        fragState.entryPoint = wgpu::StringView("fs_main");
        fragState.constantCount = 0;
        fragState.constants = nullptr;

        desc.depthStencil = nullptr;

        wgpu::BlendState blendState{};

        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = m_surfaceFormat;
        colorTarget.blend = &blendState;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;

        fragState.targetCount = 1;
        fragState.targets = &colorTarget;
        desc.fragment = &fragState;


        blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
        blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
        blendState.color.operation = wgpu::BlendOperation::Add;
        blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
        blendState.alpha.dstFactor = wgpu::BlendFactor::One;
        blendState.alpha.operation = wgpu::BlendOperation::Add;


        // Samples per pixel
        desc.multisample.count = 1;
        // Default value for the mask, meaning "all bits on"
        desc.multisample.mask = ~0u;
        // Default value as well (irrelevant for count = 1 anyways)
        desc.multisample.alphaToCoverageEnabled = false;

        m_pipeline = m_device.createRenderPipeline(desc);
        shaderModule.release();
    }

    void Renderer::update() {
        auto [ surfaceTexture, targetView ] = getNextSurfaceViewData();
        if (!targetView) return;

        wgpu::CommandEncoderDescriptor encoderDesc{};
        encoderDesc.nextInChain = nullptr;
        encoderDesc.label = WGPUStringView("Frame encoder");

        auto encoder = m_device.createCommandEncoder(encoderDesc);


        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};

        renderPassColorAttachment.view = targetView;
        renderPassColorAttachment.resolveTarget = nullptr;

        renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
        renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
        renderPassColorAttachment.clearValue = wgpu::Color{0.9, 0.1, 0.2, 1.0};
        renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.timestampWrites = nullptr;

        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        renderPass.setPipeline(m_pipeline);
        renderPass.draw(3, 1, 0, 0);

        renderPass.end();
        renderPass.release();

        wgpu::CommandBufferDescriptor cmdDesc = {};
        wgpu::CommandBuffer cmd = encoder.finish(cmdDesc);


        // Finally submit the command queue

        m_queue.submit(cmd);
        cmd.release();
        encoder.release();

        m_surface.present();
        targetView.release();
        m_device.poll(false, nullptr);
    }

}
