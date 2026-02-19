#pragma once

#include "Resources/ResourceParam.h"
#include "./Components/ShaderManager.h"
#include "Window.h"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>


namespace crg::renderer {

    struct RenderContext {
        wgpu::Instance instance;

        wgpu::Surface surface;
        wgpu::SurfaceCapabilities capabilities;
        wgpu::TextureFormat surfaceFormat;

        wgpu::Sampler sampler;

        wgpu::Device device;
        wgpu::Adapter adapter;
        wgpu::Queue queue;
        wgpu::SurfaceConfiguration config;
        Window* window;

        wgpu::RenderPipeline pipeline;
        wgpu::PipelineLayout pipelineLayout;

        // wgpu::BindGroupLayout bindGroupLayout;

        // wgpu::VertexBufferLayout vertexLayout;

        wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
        wgpu::Texture depthTexture;
        wgpu::TextureView depthView;

        RenderContext(Window* window);


    private:
        bool initInstance();

        bool initSurface();

        void getAdapter();

        void initDevice();

        void initDepthBuffer();



        wgpu::Limits getRequiredLimits();
    };


    void initPipeline(
        ecs::Res<ShaderManager>,
        ecs::ResMut<RenderContext>
    );



}
