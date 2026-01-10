#pragma once

#include "Window.h"
#include <cstdint>
#define WEBGPU_CPP_IMPLEMENTATIONM

#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    class Renderer {
    public:
        Renderer(Window* window);
        ~Renderer();

        void update();

    private:
        void fetchInstance();

        void getSurface();

        void configureSurface();

        void fetchAdapter();

        void printAdapterInfo();

        void fetchDevice();

        void fetchQueue();

        void makePipeline();

        std::pair<wgpu::SurfaceTexture, wgpu::TextureView> getNextSurfaceViewData();

        wgpu::Limits getRequiredLimits() const;

        void initializeBuffers();
        void initializeBindings();
        void createBindGroupLayout();
    private:

        Window* m_window;

        wgpu::Instance m_instance;
        wgpu::Surface m_surface;
        wgpu::TextureFormat m_surfaceFormat = wgpu::TextureFormat::Undefined;
        wgpu::Device m_device;
        wgpu::Adapter m_adapter;
        wgpu::Queue m_queue;
        wgpu::RenderPipeline m_pipeline;
        wgpu::Buffer m_pointBuffer;
        wgpu::Buffer m_indexBuffer;
        wgpu::Buffer m_uniformBuffer;
        wgpu::BindGroupLayout m_bindGroupLayout;
        wgpu::BindGroup m_bindGroup;
        wgpu::PipelineLayout m_layout;
        uint32_t m_indexCount = 0;

        wgpu::TextureView m_depthTextureView;
        wgpu::Texture m_depthTexture;


        uint32_t m_uniformStride = 0;

        struct MyUniform {
            std::array<float, 4> color;
            float time;
            float _padding[3];
        };
        static_assert(sizeof(MyUniform) % 16 == 0, "size of uniform not a multiple of 16");

    };

}
