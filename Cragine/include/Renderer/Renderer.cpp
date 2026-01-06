#include <cstdint>
#include <unistd.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "Renderer.h"
#include "Renderer/utils/helpers.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

namespace crg::renderer {

    const char* shaderSource = R"(
        struct VertexInput {
            @location(0) position: vec2f,
            @location(1) color: vec3f,
        };

        struct VertexOutput {
            @builtin(position) position: vec4f,

            @location(0) color: vec3f
        };

        @vertex
        fn vs_main(in: VertexInput) -> VertexOutput {
            var out: VertexOutput;
            out.position = vec4f(in.position, 0.0, 1.0);
            out.color = in.color;

           	return out;
        }

        @fragment
        fn fs_main(in: VertexOutput) -> @location(0) vec4f {
       	    return vec4f(in.color, 1.0);
        }


    )";

    std::vector<float> pointData = {
        -0.5, -0.5, // Point #0 (A)
        +0.5, -0.5, // Point #1
        +0.5, +0.5, // Point #2 (C)
        -0.5, +0.5, // Point #3
    };

    // Define index data
    // This is a list of indices referencing positions in the pointData
    std::vector<uint16_t> indexData = {
        0, 1, 2, // Triangle #0 connects points #0, #1 and #2
        0, 2, 3  // Triangle #1 connects points #0, #2 and #3
    };

    std::vector<float> colorData = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 0.0,
        1.0, 0.0, 1.0,
        0.0, 1.0, 1.0
    };


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

        initializeBuffers();
    }

    Renderer::~Renderer() {
        m_indexBuffer.release();
        m_pointBuffer.release();
        m_colorBuffer.release();
        LOG_CORE_INFO("Released wgpu buffers");

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
            LOG_CORE_INFO(" - maxVertexAttributes: {}", supportedLimits.maxVertexAttributes);

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
        auto limits = getRequiredLimits();
        deviceDescriptor.requiredLimits = &limits;


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

        wgpu::Limits deviceLimits{};
        m_device.getLimits(&deviceLimits);
        LOG_CORE_INFO("Device vertex attribute: {}", deviceLimits.maxVertexAttributes);
    }


    wgpu::Limits Renderer::getRequiredLimits() const {
        wgpu::Limits supportedLimits;
        m_adapter.getLimits(&supportedLimits);

        wgpu::Limits requiredLimits = wgpu::Default;
        requiredLimits.maxVertexAttributes = 2;
        requiredLimits.maxVertexBuffers = 2;
        requiredLimits.maxBufferSize = 6 * 5 * sizeof(float);
        requiredLimits.maxVertexBufferArrayStride = 5 * sizeof(float);
        requiredLimits.maxInterStageShaderVariables = 3;

        return requiredLimits;
    }

    void Renderer::initializeBuffers() {
        wgpu::BufferDescriptor bufferDesc{};

        // Vertex buffer
        bufferDesc.size = pointData.size() * sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
        bufferDesc.mappedAtCreation = false;
        m_pointBuffer = m_device.createBuffer(bufferDesc);
        m_queue.writeBuffer(m_pointBuffer, 0, pointData.data(), bufferDesc.size);

        // Index buffer
        m_indexCount = static_cast<uint32_t>(indexData.size());

        bufferDesc.size = indexData.size() * sizeof(uint16_t);
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
        bufferDesc.size = (bufferDesc.size + 3) & ~3; // Round up to the next multiple of 4
        indexData.resize((indexData.size() + 1) & ~1); // Round up to the next multiple of 2
        m_indexBuffer = m_device.createBuffer(bufferDesc);

        m_queue.writeBuffer(m_indexBuffer, 0, indexData.data(), bufferDesc.size);


        bufferDesc.label = wgpu::StringView("Vertex Color");
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
        bufferDesc.size = colorData.size() * sizeof(float);
        m_colorBuffer = m_device.createBuffer(bufferDesc);
        m_queue.writeBuffer(m_colorBuffer, 0, colorData.data(), bufferDesc.size);
        m_vertexCount = static_cast<uint32_t>(pointData.size() / 2);
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

        std::vector<wgpu::VertexBufferLayout> vertexBufferLayouts(2);

        std::vector<wgpu::VertexAttribute> vertAttribs(2);
        // Corresponds to @location(0)
        vertAttribs[0].shaderLocation = 0;
        vertAttribs[0].format = wgpu::VertexFormat::Float32x2;
        vertAttribs[0].offset = 0;

        vertAttribs[1].shaderLocation = 1;
        vertAttribs[1].format = wgpu::VertexFormat::Float32x3;
        vertAttribs[1].offset = 2 * sizeof(float);

        wgpu::VertexAttribute posAttrib{};
        posAttrib.shaderLocation = 0;
        posAttrib.format = wgpu::VertexFormat::Float32x2;
        posAttrib.offset = 0;

        vertexBufferLayouts[0].attributeCount = 1;
        vertexBufferLayouts[0].attributes = &posAttrib;
        vertexBufferLayouts[0].arrayStride = 2 * sizeof(float);
        vertexBufferLayouts[0].stepMode = wgpu::VertexStepMode::Vertex;

        wgpu::VertexAttribute colorAttrib{};
        colorAttrib.shaderLocation = 1;
        colorAttrib.format = wgpu::VertexFormat::Float32x3;
        colorAttrib.offset = 0;

        vertexBufferLayouts[1].attributeCount = 1;
        vertexBufferLayouts[1].attributes = &colorAttrib;
        vertexBufferLayouts[1].arrayStride = 3 * sizeof(float);
        vertexBufferLayouts[1].stepMode = wgpu::VertexStepMode::Vertex;


        desc.vertex.bufferCount = static_cast<uint32_t>(vertexBufferLayouts.size());
        desc.vertex.buffers = vertexBufferLayouts.data();

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

        renderPass.setVertexBuffer(0, m_pointBuffer, 0, m_pointBuffer.getSize());

        renderPass.setIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint16, 0, m_indexBuffer.getSize());

        renderPass.setVertexBuffer(1, m_colorBuffer, 0, m_colorBuffer.getSize());

        renderPass.drawIndexed(m_indexCount, 1, 0, 0, 0);

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
