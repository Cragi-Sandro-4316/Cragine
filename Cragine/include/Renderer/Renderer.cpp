#include <cmath>
#include <cstddef>
#include <cstdint>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <type_traits>
#include <unistd.h>
#include <webgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "Renderer.h"
#include "Renderer/utils/helpers.h"
#include "utils/Logger.h"
#include "utils/Loader.h"
#include <GLFW/glfw3.h>
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


        initializeBuffers();
        createBindGroupLayout();
        initializeBindings();

        makePipeline();
    }

    Renderer::~Renderer() {
        m_depthTextureView.release();
        m_depthTexture.destroy();
        m_depthTexture.release();

        m_uniform.release();
        m_vertexData.release();
        LOG_CORE_INFO("Released wgpu buffers");


        m_layout.release();
        m_bindGroupLayout.release();
        m_bindGroup.release();
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
        requiredLimits.maxVertexAttributes = 3;
        requiredLimits.maxVertexBuffers = 1;
        requiredLimits.maxBufferSize = supportedLimits.maxBufferSize;
        requiredLimits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
        requiredLimits.maxInterStageShaderVariables = 6;
        requiredLimits.maxBindGroups = 1;
        requiredLimits.maxUniformBuffersPerShaderStage = 1;
        requiredLimits.maxUniformBufferBindingSize = supportedLimits.maxUniformBufferBindingSize;
        requiredLimits.maxDynamicUniformBuffersPerPipelineLayout = 1;

        requiredLimits.maxTextureDimension1D = 480;
        requiredLimits.maxTextureDimension2D = 8192;
        requiredLimits.maxTextureArrayLayers = 1;


        return requiredLimits;
    }

    void Renderer::initializeBuffers() {

        // Load mesh data
        std::vector<float> pointData;
        std::vector<uint16_t> indexData;

        LOG_CORE_INFO("Location: {}", RESOURCE_DIR "/model.txt");
        bool success = ModelLoader::loadGeometry(RESOURCE_DIR "/model.txt", pointData, indexData, 6);

        if (!success) {
            LOG_CORE_ERROR("Failed to load geometry");
            exit(1);
        }

        m_vertexData = std::make_unique<VertexData>(m_device, m_queue, pointData, indexData);

        m_uniform = std::make_unique<Uniform<MyUniform>>(m_device, m_queue);


        wgpu::BufferDescriptor bufferDesc{};

        MyUniform uniform;

        // Upload first value
        uniform.time = 1.0f;
        uniform.color = glm::vec4(1.);

        m_uniform->update(uniform);
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

        std::vector<wgpu::VertexAttribute> vertAttribs(3);

        // Corresponds to @location(0) position
        vertAttribs[0].shaderLocation = 0;
        vertAttribs[0].format = wgpu::VertexFormat::Float32x3;
        vertAttribs[0].offset = offsetof(VertexAttributes, position);

        // Corresponds to @location(1) normal
        vertAttribs[1].shaderLocation = 1;
        vertAttribs[1].format = wgpu::VertexFormat::Float32x3;
        vertAttribs[1].offset = offsetof(VertexAttributes, normal);

        // Corresponds to @location(2) color
        vertAttribs[2].shaderLocation = 2;
        vertAttribs[2].format = wgpu::VertexFormat::Float32x3;
        vertAttribs[2].offset = offsetof(VertexAttributes, color);


        auto& layout = m_vertexData->layout();



        layout.attributeCount = static_cast<uint32_t>(vertAttribs.size());
        layout.attributes = vertAttribs.data();
        layout.arrayStride = sizeof(VertexAttributes);
        layout.stepMode = wgpu::VertexStepMode::Vertex;

        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &layout;

        LOG_CORE_INFO("Loading shader...");
        wgpu::ShaderModule shaderModule = ModelLoader::loadShader(RESOURCE_DIR"/shader.wgsl", m_device);
        LOG_CORE_INFO("shader module: {}", (void*)shaderModule);
        if (!shaderModule) {
            LOG_CORE_ERROR("Could not load shader");
            exit(1);
        }

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


        desc.depthStencil = nullptr;
        // Samples per pixel
        desc.multisample.count = 1;
        // Default value for the mask, meaning "all bits on"
        desc.multisample.mask = ~0u;
        // Default value as well (irrelevant for count = 1 anyways)
        desc.multisample.alphaToCoverageEnabled = false;


    	// Create the pipeline layout
    	wgpu::PipelineLayoutDescriptor layoutDesc{};
    	layoutDesc.bindGroupLayoutCount = 1;
        WGPUBindGroupLayout rawLayout = m_bindGroupLayout;
    	layoutDesc.bindGroupLayouts = &rawLayout;
    	m_layout = m_device.createPipelineLayout(layoutDesc);

        desc.layout = m_layout;

        wgpu::DepthStencilState depthStencilState = wgpu::Default;

        depthStencilState.depthCompare = wgpu::CompareFunction::Less;
        depthStencilState.depthWriteEnabled = wgpu::OptionalBool::True;
        wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
        depthStencilState.format = depthTextureFormat;
        depthStencilState.stencilReadMask = 0;
        depthStencilState.stencilWriteMask = 0;

        wgpu::TextureDescriptor depthTextureDesc;
        depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
        depthTextureDesc.format = depthTextureFormat;
        depthTextureDesc.mipLevelCount = 1;
        depthTextureDesc.sampleCount = 1;
        depthTextureDesc.size = {m_window->getWidth(), m_window->getHeight(), 1};
        depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
        depthTextureDesc.viewFormatCount = 1;
        depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
        m_depthTexture = m_device.createTexture(depthTextureDesc);


        wgpu::TextureViewDescriptor depthTextureViewDesc;
        depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
        depthTextureViewDesc.baseArrayLayer = 0;
        depthTextureViewDesc.arrayLayerCount = 1;
        depthTextureViewDesc.baseMipLevel = 0;
        depthTextureViewDesc.mipLevelCount = 1;
        depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
        depthTextureViewDesc.format = depthTextureFormat;
        m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);



        desc.depthStencil = &depthStencilState;


        m_pipeline = m_device.createRenderPipeline(desc);
        shaderModule.release();

    }


    void Renderer::createBindGroupLayout() {
        wgpu::BindGroupLayoutEntry bindingLayout{};
        bindingLayout.binding = 0;  // Binding(0) in wgsl
        bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
        bindingLayout.buffer.minBindingSize = sizeof(MyUniform);

        bindingLayout.buffer.hasDynamicOffset = true;

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
        bindGroupLayoutDesc.entryCount = 1;
        bindGroupLayoutDesc.entries = &bindingLayout;

        m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);
    }


    void Renderer::initializeBindings() {
        // Create a binding
    	wgpu::BindGroupEntry binding{};
    	// The index of the binding (the entries in bindGroupDesc can be in any order)
    	binding.binding = 0;
    	// The buffer it is actually bound to
    	binding.buffer = m_uniform->getBuffer();
    	// We can specify an offset within the buffer, so that a single buffer can hold
    	// multiple uniform blocks.
    	binding.offset = 0;
    	// And we specify again the size of the buffer.
    	binding.size = sizeof(MyUniform);

    	// A bind group contains one or multiple bindings
    	wgpu::BindGroupDescriptor bindGroupDesc{};
    	bindGroupDesc.layout = m_bindGroupLayout;
    	// There must be as many bindings as declared in the layout!
    	bindGroupDesc.entryCount = 1;
    	bindGroupDesc.entries = &binding;
    	m_bindGroup = m_device.createBindGroup(bindGroupDesc);
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
        renderPassColorAttachment.clearValue = wgpu::Color{0.1, 0.1, 0.1, 1.0};
        renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;

        depthStencilAttachment.view = m_depthTextureView;

        // The initial value of the depth buffer, meaning "far"
        depthStencilAttachment.depthClearValue = 1.0f;
        // Operation settings comparable to the color attachment
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
        // we could turn off writing to the depth buffer globally here
        depthStencilAttachment.depthReadOnly = false;

        // Stencil setup, mandatory but unused
        depthStencilAttachment.stencilClearValue = 0;
        depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.stencilReadOnly = true;


        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.timestampWrites = nullptr;

        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        renderPass.setPipeline(m_pipeline);

        renderPass.setVertexBuffer(0, m_vertexData->vertexBuffer(), 0, m_vertexData->vertexBuffer().getSize());

        renderPass.setIndexBuffer(m_vertexData->indexBuffer(), wgpu::IndexFormat::Uint16, 0, m_vertexData->indexBuffer().getSize());


        auto uniformData = uniformOps();

        m_uniform->update(uniformData);


        uint32_t offset = 0;
        renderPass.setBindGroup(0, m_bindGroup, 1, &offset);
        renderPass.drawIndexed(m_vertexData->indexCount(), 1, 0, 0, 0);

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


    // TODO: temporary function to avoid boilerplate
    Renderer::MyUniform Renderer::uniformOps() {

        MyUniform uniformData{};
        float t = static_cast<float>(glfwGetTime());
        glm::vec4 color = {
            std::abs(std::sin(t)),
            std::abs(std::sin(t)),
            0.f,
            1.f
        };
        // Scale the object
        glm::mat4x4 S = transpose(glm::mat4x4(
            0.3,  0.0, 0.0, 0.0,
            0.0,  0.3, 0.0, 0.0,
            0.0,  0.0, 0.3, 0.0,
            0.0,  0.0, 0.0, 1.0
        ));

        // Translate the object
        glm::mat4x4 T1 = transpose(glm::mat4x4(
            1.0,  0.0, 0.0, 0.5,
            0.0,  1.0, 0.0, 0.0,
            0.0,  0.0, 1.0, 0.0,
            0.0,  0.0, 0.0, 1.0
        ));

        // Rotate the object
        float angle1 = (float)glfwGetTime();
        float c1 = cos(angle1);
        float s1 = sin(angle1);
        glm::mat4x4 R1 = transpose(glm::mat4x4(
             c1,  s1, 0.0, 0.0,
            -s1,  c1, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        ));

        // Translate the view
        glm::vec3 focalPoint(0.0, 0.0, -2.0);
        glm::mat4x4 T2 = transpose(glm::mat4x4(
            1.0,  0.0, 0.0, -focalPoint.x,
            0.0,  1.0, 0.0, -focalPoint.y,
            0.0,  0.0, 1.0, -focalPoint.z,
            0.0,  0.0, 0.0,     1.0
        ));

        // Rotate the view point
        float angle2 = 3.0 * PI / 4.0;
        float c2 = cos(angle2);
        float s2 = sin(angle2);
        glm::mat4x4 R2 = transpose(glm::mat4x4(
            1.0, 0.0, 0.0, 0.0,
            0.0,  c2,  s2, 0.0,
            0.0, -s2,  c2, 0.0,
            0.0, 0.0, 0.0, 1.0
        ));

        float ratio = 640.0f / 480.0f;
        float focalLength = 2.0;
        float near = 0.01f;
        float far = 100.0f;
        float divider = 1 / (focalLength * (far - near));
        uniformData.projectionMatrix = transpose(glm::mat4x4(
            1.0, 0.0, 0.0, 0.0,
            0.0, ratio, 0.0, 0.0,
            0.0, 0.0, far * divider, -far * near * divider,
            0.0, 0.0, 1.0 / focalLength, 0.0
        ));



        uniformData.viewMatrix = T2 * R2;

        uniformData.time = t;
        uniformData.color = color;
        uniformData.modelMatrix = R1 * T1 * S;
        return uniformData;
    }

}
