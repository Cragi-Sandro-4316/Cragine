#pragma once

#include "Window.h"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#define WEBGPU_CPP_IMPLEMENTATIONM

#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>

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
        const float PI = 3.14159265358979323846f;

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

            glm::mat4x4 projectionMatrix;
            glm::mat4x4 viewMatrix;
            glm::mat4x4 modelMatrix;

            glm::vec4 color;
            float time;
            float _padding[3];
        };
        static_assert(sizeof(MyUniform) % 16 == 0, "size of uniform not a multiple of 16");

    };

}
