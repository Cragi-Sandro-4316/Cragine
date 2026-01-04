#pragma once

#include "Window.h"
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

    private:

        Window* m_window;

        wgpu::Instance m_instance;
        wgpu::Surface m_surface;
        wgpu::TextureFormat m_surfaceFormat = wgpu::TextureFormat::Undefined;
        wgpu::Device m_device;
        wgpu::Adapter m_adapter;
        wgpu::Queue m_queue;
        wgpu::RenderPipeline m_pipeline;

    };

}
