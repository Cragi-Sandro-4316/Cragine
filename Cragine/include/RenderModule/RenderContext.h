#pragma once

#include "RenderModule/Structs/DepthTexture.h"
#include "Window.h"
#include <webgpu/webgpu.hpp>
namespace crg::renderer {

    struct RenderContext {

        RenderContext(Window* window);

        wgpu::Instance instance;

        wgpu::Surface surface;
        wgpu::SurfaceCapabilities capabilities;
        wgpu::TextureFormat surfaceFormat;

        DepthTexture depthTexture;
        wgpu::DepthStencilState depthStencilState;
        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;

        wgpu::Device device;
        wgpu::Adapter adapter;
        wgpu::Queue queue;
        wgpu::SurfaceConfiguration config;
        Window* window;

    private:
        bool initInstance();

        bool initSurface();

        void getAdapter();

        void initDevice();

        void initDepthBuffer();

        wgpu::Limits getRequiredLimits();
    };

}
