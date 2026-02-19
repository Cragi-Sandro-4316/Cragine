#include "RenderContext.h"
#include "Rendererr/FrameData.h"
#include "Resources/ResourceParam.h"
#include "glfw3webgpu.h"
#include "utils/Logger.h"
#include "./utils/Loader.h"
#include "./Components/MeshPool.h"
#include "./Components/ShaderManager.h"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    RenderContext::RenderContext(Window* window)
    : window(window) {
        if (!initInstance()) return;

        if (!initSurface()) return;

        getAdapter();

        initDevice();
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

        initDepthBuffer();


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
        sampler = device.createSampler(samplerDesc);
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


        device = adapter.requestDevice(deviceDescriptor);
    }


    void RenderContext::initDepthBuffer() {
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
       	depthTexture = device.createTexture(depthTextureDesc);
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
       	depthView = depthTexture.createView(depthTextureViewDesc);
       	std::cout << "Depth texture view: " << depthView << std::endl;
    }

    void initPipeline(
        ecs::Res<ShaderManager> shaderManagerRes,
        ecs::ResMut<RenderContext> renderContextRes
    ) {
        auto& shaderManager = shaderManagerRes.get();
        auto& renderContext = renderContextRes.get();

        auto& shader = shaderManager.addShader(
            renderContext.device,
            renderContext.depthTextureFormat,
            renderContext.surfaceFormat
        );


        wgpu::RenderPipelineDescriptor desc{};

        desc.vertex.module = shader.shaderModule;
        desc.vertex.entryPoint = wgpu::StringView("vs_main");
        desc.vertex.constantCount = 0;
        desc.vertex.constants = nullptr;
        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &shader.vertLayout;

        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
        desc.primitive.frontFace = wgpu::FrontFace::CCW;
        desc.primitive.cullMode = wgpu::CullMode::None;

        desc.depthStencil = &shader.depthStencilState;

        desc.fragment = &shader.fragState;

        // Samples per pixel
        desc.multisample.count = 1;
        // Default value for the mask, meaning "all bits on"
        desc.multisample.mask = ~0u;
        // Default value as well (irrelevant for count = 1 anyways)
        desc.multisample.alphaToCoverageEnabled = false;


        // Create the pipeline layout
       	wgpu::PipelineLayoutDescriptor layoutDesc{};
       	layoutDesc.bindGroupLayoutCount = 1;
       	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&shader.bindGroupLayout;
       	renderContext.pipelineLayout = renderContext.device.createPipelineLayout(layoutDesc);

        desc.layout = renderContext.pipelineLayout;


        LOG_CORE_ERROR("vert step mode: {}", (int)renderContext.depthTextureFormat);
        renderContext.pipeline = renderContext.device.createRenderPipeline(desc);
    }

}
