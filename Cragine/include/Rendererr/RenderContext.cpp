#include "RenderContext.h"
#include "glfw3webgpu.h"
#include "utils/Logger.h"
#include "./utils/Loader.h"
#include "./Components/MeshPool.h"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {
    wgpu::Instance initInstance() {
        wgpu::InstanceDescriptor desc{};
        desc.nextInChain = nullptr;

        auto instance = wgpu::createInstance(desc);

        if (!instance) {
            LOG_CORE_ERROR("Could not initialize wgpu");
        }

        return instance;
    }

    wgpu::Surface initSurface(wgpu::Instance instance, Window* window) {
        auto surface = glfwCreateWindowWGPUSurface(instance, window->getGlfwWindow());
        if (!surface) {
            LOG_CORE_ERROR("Could not get surface");
        }

        return surface;
    }

    wgpu::Adapter getAdapter(wgpu::Instance instance, wgpu::Surface surface) {
        wgpu::RequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        adapterOptions.compatibleSurface = surface;

        return instance.requestAdapter(adapterOptions);
    }

    wgpu::Limits getRequiredLimits(wgpu::Adapter adapter) {
        wgpu::Limits supportedLimits;
        adapter.getLimits(&supportedLimits);

        wgpu::Limits requiredLimits = supportedLimits;
        return requiredLimits;
    }

    wgpu::Device initDevice(wgpu::Adapter adapter) {

        auto limits = getRequiredLimits(adapter);

        wgpu::DeviceDescriptor deviceDescriptor{};
        deviceDescriptor.label = WGPUStringView("Device");
        deviceDescriptor.nextInChain = nullptr;
        deviceDescriptor.requiredFeatureCount = 0;
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


        return adapter.requestDevice(deviceDescriptor);
    }


    std::pair<wgpu::Texture, wgpu::TextureView> initDepthBuffer(
        wgpu::TextureFormat depthTextureFormat,
        wgpu::Device device,
        Window* window
    ) {
        // Create the depth texture
    	wgpu::TextureDescriptor depthTextureDesc;
    	depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
    	depthTextureDesc.format = depthTextureFormat;
    	depthTextureDesc.mipLevelCount = 1;
    	depthTextureDesc.sampleCount = 1;
    	depthTextureDesc.size = {
            window->getWidth(),
            window->getHeight(),
            1
        };
    	depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    	depthTextureDesc.viewFormatCount = 1;
    	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
    	auto depthTexture = device.createTexture(depthTextureDesc);
    	std::cout << "Depth texture: " << depthTexture << std::endl;

    	// Create the view of the depth texture manipulated by the rasterizer
    	wgpu::TextureViewDescriptor depthTextureViewDesc;
    	depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    	depthTextureViewDesc.baseArrayLayer = 0;
    	depthTextureViewDesc.arrayLayerCount = 1;
    	depthTextureViewDesc.baseMipLevel = 0;
    	depthTextureViewDesc.mipLevelCount = 1;
    	depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    	depthTextureViewDesc.format = depthTextureFormat;
    	auto depthTextureView = depthTexture.createView(depthTextureViewDesc);
    	std::cout << "Depth texture view: " << depthTextureView << std::endl;

    	return {depthTexture, depthTextureView};
    }


    struct UniformStruct{

        glm::mat4x4 projectionMatrix;
        glm::mat4x4 viewMatrix;
        glm::mat4x4 modelMatrix;

        glm::vec4 color;
        float time;
        float _padding[3];
    };

    auto createBindGroupLayout(wgpu::Device& device) {
        std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(3);
        auto& bindingLayout = bindingLayoutEntries[0];
        bindingLayout.binding = 0;  // Binding(0) in wgsl
        bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
        bindingLayout.buffer.minBindingSize = sizeof(UniformStruct);
        bindingLayout.buffer.hasDynamicOffset = false;

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

        return device.createBindGroupLayout(bindGroupLayoutDesc);
    }



    std::tuple<wgpu::RenderPipeline, wgpu::PipelineLayout, wgpu::BindGroupLayout> initPipeline(
        wgpu::Device& device,
        wgpu::VertexBufferLayout& vertexLayout,
        wgpu::TextureFormat& depthTextureFormat,
        wgpu::TextureFormat& surfaceFormat
    ) {
        auto shaderModule = Loader::loadShader(RESOURCE_DIR "/shader.wgsl", device);

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

        vertexLayout.attributeCount = static_cast<uint32_t>(vertAttribs.size());
        vertexLayout.attributes = vertAttribs.data();
        vertexLayout.arrayStride = sizeof(VertexAttributes);
        vertexLayout.stepMode = wgpu::VertexStepMode::Vertex;

        desc.vertex.module = shaderModule;
        desc.vertex.entryPoint = wgpu::StringView("vs_main");
        desc.vertex.constantCount = 0;
        desc.vertex.constants = nullptr;
        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &vertexLayout;

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
        depthStencilState.format = depthTextureFormat;
        depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
        depthStencilState.depthCompare = wgpu::CompareFunction::Less;

        desc.depthStencil = &depthStencilState;

        wgpu::BlendState blendState{};

        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = surfaceFormat;
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

        wgpu::BindGroupLayout bindGroupLayout = createBindGroupLayout(device);

        // Create the pipeline layout
       	wgpu::PipelineLayoutDescriptor layoutDesc{};
       	layoutDesc.bindGroupLayoutCount = 1;
        WGPUBindGroupLayout rawLayout = bindGroupLayout;
       	layoutDesc.bindGroupLayouts = &rawLayout;
       	auto layout = device.createPipelineLayout(layoutDesc);

        desc.layout = layout;

        return { device.createRenderPipeline(desc), layout, bindGroupLayout };
    }

    RenderContext::RenderContext(Window* window)
    : window(window) {
        instance = initInstance();
        if (!instance) return;

        surface = initSurface(instance, window);
        if (!surface) return;


        auto adapter = getAdapter(instance, surface);
        if (!adapter) return;

        device = initDevice(adapter);
        queue = device.getQueue();

        config = wgpu::SurfaceConfiguration{};
        config.nextInChain = nullptr;
        config.width = window->getWidth();
        config.height = window->getHeight();


        surface.getCapabilities(adapter, &capabilities);

        surfaceFormat = capabilities.formats[0];
        config.format = capabilities.formats[0];
        config.viewFormatCount = 0;
        config.viewFormats = nullptr;

        config.usage = WGPUTextureUsage_RenderAttachment;
        config.device = device;
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Auto;

        surface.configure(config);




        std::tie(depthTexture, depthView) = initDepthBuffer(
            depthTextureFormat,
            device,
            window
        );



        std::tie(pipeline, pipelineLayout, bindGroupLayout) = initPipeline(
            device,
            vertexLayout,
            depthTextureFormat,
            surfaceFormat
        );

    }


}
