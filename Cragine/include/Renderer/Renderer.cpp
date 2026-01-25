#include <cmath>
#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
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
        if (!initDevice()) return;
        // if (!initSwapchain()) return;
        if (!initDepthBuffer()) return;
        if (!initRenderPipeline()) return;
        if (!initTexture()) return;
        if (!initGeometry()) return;
        if (!initUniforms()) return;
        if (!initBindGroup()) return;


        // Set window resize callback
        glfwSetWindowUserPointer(m_window->getGlfwWindow(), this);

        glfwSetFramebufferSizeCallback(m_window->getGlfwWindow(), [](GLFWwindow* window, int, int) {
            auto that = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
            if (that) that->onResize();
        });
    }


    void Renderer::onResize() {
        terminateDepthBuffer();
        terminateSwapchain();

        initDepthBuffer();
    }



    Renderer::~Renderer() {
        if (m_device) {
            m_device.poll(true, nullptr);
        }


        m_layout.release();
        m_bindGroupLayout.release();
        m_bindGroup.release();
        m_pipeline.release();
        LOG_CORE_INFO("Released wgpu pipeline");
        m_depthTextureView.release();
        m_textureView.release();
        m_depthTexture.destroy();
        m_texture.release();
        m_depthTexture.release();
        // m_texture.destroy();
        m_uniform.release();
        m_vertexBuffer.release();
        LOG_CORE_INFO("Released wgpu buffers");
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





    bool Renderer::getSurface() {
        m_surface = glfwCreateWindowWGPUSurface(m_instance, m_window->getGlfwWindow());
        if (!m_surface) {
            LOG_CORE_ERROR("Could not initialize wgpu surface!");
            return false;
        }
        return true;
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


    bool Renderer::fetchAdapter() {
        // Gets the gpu adapter
        LOG_CORE_INFO("Requesting adapter...");
        if (!getSurface()) {
            return false;
        }

        wgpu::RequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        adapterOptions.compatibleSurface = m_surface;
        m_adapter = m_instance.requestAdapter(adapterOptions);

        LOG_CORE_INFO("Got adapter: {}", (void*)m_adapter);
        return true;
    }


    bool Renderer::initDevice() {
        fetchInstance();

        if (!m_instance) {
            LOG_CORE_ERROR("Could not initialize webgpu");
            return false;
        }

        fetchAdapter();

        auto limits = getRequiredLimits();


        LOG_CORE_INFO("Requesting device...");

        wgpu::DeviceDescriptor deviceDescriptor{};
        deviceDescriptor.label = WGPUStringView("Device");
        deviceDescriptor.nextInChain = nullptr;
        deviceDescriptor.requiredFeatureCount = 0;
        deviceDescriptor.requiredLimits = &limits;
        deviceDescriptor.defaultQueue.label = WGPUStringView("The default queue");
        deviceDescriptor.defaultQueue.nextInChain = nullptr;


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


        m_device = m_adapter.requestDevice(deviceDescriptor);
        LOG_CORE_INFO("Got device: {}", (void*)m_device);

        m_queue = m_device.getQueue();

        wgpu::Limits deviceLimits{};
        m_device.getLimits(&deviceLimits);
        LOG_CORE_INFO("Device vertex attribute: {}", deviceLimits.maxVertexAttributes);

        configureSurface();
        return true;
    }


    void Renderer::fetchInstance() {
        wgpu::InstanceDescriptor desc{};
        desc.nextInChain = nullptr;

        m_instance = wgpuCreateInstance(&desc);
    }

    bool Renderer::initDepthBuffer() {
        // Create the depth texture
    	wgpu::TextureDescriptor depthTextureDesc;
    	depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
    	depthTextureDesc.format = m_depthTextureFormat;
    	depthTextureDesc.mipLevelCount = 1;
    	depthTextureDesc.sampleCount = 1;
    	depthTextureDesc.size = {
            m_window->getWidth(),
            m_window->getHeight(),
            1
        };
    	depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    	depthTextureDesc.viewFormatCount = 1;
    	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;
    	m_depthTexture = m_device.createTexture(depthTextureDesc);
    	std::cout << "Depth texture: " << m_depthTexture << std::endl;

    	// Create the view of the depth texture manipulated by the rasterizer
    	wgpu::TextureViewDescriptor depthTextureViewDesc;
    	depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    	depthTextureViewDesc.baseArrayLayer = 0;
    	depthTextureViewDesc.arrayLayerCount = 1;
    	depthTextureViewDesc.baseMipLevel = 0;
    	depthTextureViewDesc.mipLevelCount = 1;
    	depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    	depthTextureViewDesc.format = m_depthTextureFormat;
    	m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);
    	std::cout << "Depth texture view: " << m_depthTextureView << std::endl;

    	return m_depthTextureView != nullptr;
    }

    bool Renderer::initRenderPipeline() {
        LOG_CORE_INFO("Creating shader module...");
        m_shaderModule = ModelLoader::loadShader(RESOURCE_DIR "/shader.wgsl", m_device);
        LOG_CORE_INFO("Shader module: {}", (void*)m_shaderModule);

        LOG_CORE_INFO("Creating pipeline");
        wgpu::RenderPipelineDescriptor desc{};

        std::vector<wgpu::VertexAttribute> vertAttribs(4);

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

        // Corresponds to @location(3) uv
        vertAttribs[3].shaderLocation = 3;
        vertAttribs[3].format = wgpu::VertexFormat::Float32x2;
        vertAttribs[3].offset = offsetof(VertexAttributes, uv);




        m_vertexLayout.attributeCount = static_cast<uint32_t>(vertAttribs.size());
        m_vertexLayout.attributes = vertAttribs.data();
        m_vertexLayout.arrayStride = sizeof(VertexAttributes);
        m_vertexLayout.stepMode = wgpu::VertexStepMode::Vertex;

        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &m_vertexLayout;

        LOG_CORE_INFO("Loading shader...");
        wgpu::ShaderModule shaderModule = ModelLoader::loadShader(RESOURCE_DIR"/shader.wgsl", m_device);
        LOG_CORE_INFO("shader module: {}", (void*)shaderModule);
        if (!shaderModule) {
            LOG_CORE_ERROR("Could not load shader");
            return false;
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


        wgpu::DepthStencilState depthStencilState{};
        depthStencilState.format = m_depthTextureFormat;
        depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
        depthStencilState.depthCompare = wgpu::CompareFunction::Less;

        desc.depthStencil = &depthStencilState;

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


        createBindGroupLayout();


    	// Create the pipeline layout
    	wgpu::PipelineLayoutDescriptor layoutDesc{};
    	layoutDesc.bindGroupLayoutCount = 1;
        WGPUBindGroupLayout rawLayout = m_bindGroupLayout;
    	layoutDesc.bindGroupLayouts = &rawLayout;
    	m_layout = m_device.createPipelineLayout(layoutDesc);

        desc.layout = m_layout;


        m_pipeline = m_device.createRenderPipeline(desc);
        shaderModule.release();
        return true;
    }


    bool Renderer::initTexture() {
        // Create a sampler
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.addressModeU = wgpu::AddressMode::Repeat;
        samplerDesc.addressModeV = wgpu::AddressMode::MirrorRepeat;
        samplerDesc.addressModeW = wgpu::AddressMode::ClampToEdge;
        samplerDesc.magFilter = wgpu::FilterMode::Linear;
        samplerDesc.minFilter = wgpu::FilterMode::Linear;
        samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
        samplerDesc.lodMinClamp = 0.0f;
        samplerDesc.lodMaxClamp = 8.0f;
        samplerDesc.compare = wgpu::CompareFunction::Undefined;
        samplerDesc.maxAnisotropy = 1;
        m_sampler = m_device.createSampler(samplerDesc);



        wgpu::TextureDescriptor textureDesc;
        textureDesc.dimension = wgpu::TextureDimension::_2D;
        textureDesc.size = { 256, 256, 1 };
        textureDesc.mipLevelCount = 8;
        textureDesc.sampleCount = 1;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
        textureDesc.viewFormatCount = 0;
        textureDesc.viewFormats = nullptr;

        // Create a texture
        m_texture = ModelLoader::loadTexture(RESOURCE_DIR "/fourareen2K_albedo.jpg", m_device, &m_textureView);
        if (!m_texture) {
            LOG_CORE_ERROR("Failed to load texture");
            return false;
        }
        LOG_CORE_INFO("Texture: {}", (void*)&m_texture);
        LOG_CORE_INFO("Texture view: {}", (void*)&m_textureView);

        return m_textureView != nullptr;
    }


    bool Renderer::initGeometry() {
        // Load mesh data
        std::vector<VertexAttributes> vertexData;

        bool success = ModelLoader::loadObjFile(RESOURCE_DIR "/fourareen.obj", vertexData);

        if (!success) {
            LOG_CORE_ERROR("Failed to load geometry");
            return false;
        }

        m_vertexBuffer = std::make_unique<VertexData>(m_device, m_queue, vertexData, m_vertexLayout);

        return m_vertexBuffer.get() != nullptr;
    }

    bool Renderer::initUniforms() {
        m_uniform = std::make_unique<Uniform<MyUniform>>(m_device, m_queue);

        MyUniform uniform;

        // Upload first value
        uniform.time = 1.0f;
        uniform.color = glm::vec4(1.);
        uniform.modelMatrix = glm::mat4x4(1.0);
        uniform.viewMatrix = glm::lookAt(glm::vec3(-0.5f, -2.5f, 2.0f), glm::vec3(0.0f), glm::vec3(0, 0, 1)); // the last argument indicates our Up direction convention
        uniform.projectionMatrix = glm::perspective(45 * PI / 180, 640.0f / 480.0f, 0.01f, 100.0f);

        m_uniform->update(uniform);

        return true;
    }

    bool Renderer::initBindGroup() {
        std::vector<wgpu::BindGroupEntry> bindings(3);

        bindings[0].binding = 0;
    	bindings[0].buffer = m_uniform->getBuffer();
    	bindings[0].offset = 0;
    	bindings[0].size = sizeof(MyUniform);

    	bindings[1].binding = 1;
    	bindings[1].textureView = m_textureView;

    	bindings[2].binding = 2;
    	bindings[2].sampler = m_sampler;

    	wgpu::BindGroupDescriptor bindGroupDesc;
    	bindGroupDesc.layout = m_bindGroupLayout;
    	bindGroupDesc.entryCount = (uint32_t)bindings.size();
    	bindGroupDesc.entries = bindings.data();
    	m_bindGroup = m_device.createBindGroup(bindGroupDesc);

    	return m_bindGroup != nullptr;
    }


    wgpu::Limits Renderer::getRequiredLimits() const {
        wgpu::Limits supportedLimits;
        m_adapter.getLimits(&supportedLimits);

        wgpu::Limits requiredLimits = wgpu::Default;
        requiredLimits.maxVertexAttributes = 4;
        requiredLimits.maxVertexBuffers = 1;
        requiredLimits.maxBufferSize = supportedLimits.maxBufferSize;
        requiredLimits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
        requiredLimits.minStorageBufferOffsetAlignment = supportedLimits.minStorageBufferOffsetAlignment;
        requiredLimits.minUniformBufferOffsetAlignment = supportedLimits.minUniformBufferOffsetAlignment;
        requiredLimits.maxInterStageShaderVariables = 8;
        requiredLimits.maxBindGroups = 1;
        requiredLimits.maxUniformBuffersPerShaderStage = 1;
        requiredLimits.maxUniformBufferBindingSize = supportedLimits.maxUniformBufferBindingSize;
        requiredLimits.maxDynamicUniformBuffersPerPipelineLayout = 1;

        requiredLimits.maxSamplersPerShaderStage = 1;

        requiredLimits.maxTextureDimension1D = supportedLimits.maxTextureDimension1D;
        requiredLimits.maxTextureDimension2D = supportedLimits.maxTextureDimension2D;
        requiredLimits.maxTextureArrayLayers = 1;
        requiredLimits.maxSampledTexturesPerShaderStage = 1;
        requiredLimits.maxSamplersPerShaderStage = 1;

        return requiredLimits;
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



    void Renderer::createBindGroupLayout() {
        std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(3);
        auto& bindingLayout = bindingLayoutEntries[0];
        bindingLayout.binding = 0;  // Binding(0) in wgsl
        bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
        bindingLayout.buffer.minBindingSize = sizeof(MyUniform);
        bindingLayout.buffer.hasDynamicOffset = true;

        auto& textureBindingLayout = bindingLayoutEntries[1];
        textureBindingLayout.binding = 1;
        textureBindingLayout.visibility = wgpu::ShaderStage::Fragment;
        textureBindingLayout.texture.sampleType = wgpu::TextureSampleType::Float;
        textureBindingLayout.texture.viewDimension = wgpu::TextureViewDimension::_2D;


        auto& samplerBindingLayout = bindingLayoutEntries[2];
        samplerBindingLayout.binding = 2;
        samplerBindingLayout.visibility = wgpu::ShaderStage::Fragment;
        samplerBindingLayout.sampler.type = wgpu::SamplerBindingType::Filtering;


        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
        bindGroupLayoutDesc.entryCount = (uint32_t)bindingLayoutEntries.size();
        bindGroupLayoutDesc.entries = bindingLayoutEntries.data();

        m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);
    }


    void Renderer::initializeBindings() {
        // Create a binding
    	std::vector<wgpu::BindGroupEntry> bindings(3);
    	bindings[0].binding = 0;
    	bindings[0].buffer = m_uniform->getBuffer();
    	bindings[0].offset = 0;
    	bindings[0].size = sizeof(MyUniform);

        bindings[1].binding = 1;
        bindings[1].textureView = m_textureView;

        bindings[2].binding = 2;
        bindings[2].sampler = m_sampler;


    	// A bind group contains one or multiple bindings
    	wgpu::BindGroupDescriptor bindGroupDesc{};
    	bindGroupDesc.layout = m_bindGroupLayout;
    	bindGroupDesc.entryCount = (uint32_t)bindings.size();
    	bindGroupDesc.entries = bindings.data();
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


        // renderPass.setIndexBuffer(m_vertexData->indexBuffer(), wgpu::IndexFormat::Uint16, 0, m_vertexData->indexBuffer().getSize());

        uint32_t idxCount = static_cast<int>(m_vertexBuffer->getVertexData().size());

        renderPass.setVertexBuffer(0, m_vertexBuffer->vertexBuffer(), 0, m_vertexBuffer->vertexBuffer().getSize());



        auto uniformData = uniformOps();

        // LOG_CORE_ERROR("Loop!");

        // float viewZ = glm::mix(0.0f, 0.25f, cos(2 * PI * glfwGetTime() / 4) * 0.5 + 0.5);
        // LOG_CORE_INFO("view z: {}", viewZ);
        // glm::mat4x4 viewMatrix = glm::lookAt(glm::vec3(-0.5f, -1.5f, viewZ + 0.25f), glm::vec3(0.0f), glm::vec3(0, 0, 1));

        // m_uniform->writeField(&viewMatrix, offsetof(MyUniform, viewMatrix));

        m_uniform->update(uniformData);

        uint32_t offset = 0;
        renderPass.setBindGroup(0, m_bindGroup, 1, &offset);
        // renderPass.drawIndexed(m_vertexData->indexCount(), 1, 0, 0, 0);

        renderPass.draw(idxCount, 1, 0, 0);


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
