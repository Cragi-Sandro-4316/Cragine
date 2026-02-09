#pragma once

#include "Renderer/BufferHelpers/Uniform.h"
#include "Renderer/BufferHelpers/VertexData.h"
#include "Window.h"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#define WEBGPU_CPP_IMPLEMENTATIONM

#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "utils/Logger.h"


namespace crg::renderer {
    class Renderer {
    public:
        Renderer(Window* window);
        ~Renderer();

        void update();


        void onResize();

    private:
        void fetchInstance();

        bool  getSurface();

        void configureSurface();

        bool fetchAdapter();

        // void printAdapterInfo();

        // void fetchDevice();

        // void fetchQueue();

        // void makePipeline();


        bool initDevice();

        bool initSwapchain();

        bool initDepthBuffer();

        bool initRenderPipeline();

        bool initTexture();

        bool initGeometry();

        bool initUniforms();

        bool initBindGroup();





        std::pair<wgpu::SurfaceTexture, wgpu::TextureView> getNextSurfaceViewData();

        wgpu::Limits getRequiredLimits() const;

        void initializeBuffers();
        void initializeBindings();
        void createBindGroupLayout();





        void terminateBindGroups() {
            LOG_CORE_INFO("Terminating bind groups");
            m_bindGroup.release();
        }

        void terminateUniforms() {
            LOG_CORE_INFO("Terminating uniforms");
            m_uniform->terminate();
        }

        void terminateGeometry() {
            LOG_CORE_INFO("Terminating geometry");
            m_vertexBuffer->terminate();
        }

        void terminateTexture() {
            LOG_CORE_INFO("Terminating textures");
            m_textureView.release();
            m_texture.destroy();
            m_texture.release();
            m_sampler.release();
        }

        void terminateRenderPipeline() {
            LOG_CORE_INFO("Terminating render pipeline");
            m_pipeline.release();
            m_shaderModule.release();
            m_bindGroupLayout.release();
        }

        void terminateDepthBuffer() {
            LOG_CORE_INFO("Terminating depth buffer");
            m_depthTextureView.release();
            m_depthTexture.destroy();
            m_depthTexture.release();
        }

        void terminateSwapchain() {
        }

        void terminateDevice() {
            LOG_CORE_INFO("Terminating device");
            m_queue.release();
            m_device.release();
            m_surface.release();
            m_instance.release();
        }

    private:
        const float PI = 3.14159265358979323846f;

        struct MyUniform {

            glm::mat4x4 projectionMatrix;
            glm::mat4x4 viewMatrix;
            glm::mat4x4 modelMatrix;

            glm::vec4 color;
            float time;
            float _padding[3];
        };
        static_assert(sizeof(MyUniform) % 16 == 0, "size of uniform not a multiple of 16");

        MyUniform uniformOps();

        Window* m_window;

        wgpu::Instance m_instance;
        wgpu::Surface m_surface;
        wgpu::TextureFormat m_surfaceFormat = wgpu::TextureFormat::Undefined;
        wgpu::Device m_device;
        wgpu::Adapter m_adapter;
        wgpu::Queue m_queue;
        wgpu::RenderPipeline m_pipeline;

        wgpu::ShaderModule m_shaderModule = nullptr;

        wgpu::VertexBufferLayout m_vertexLayout{};


        wgpu::BindGroupLayout m_bindGroupLayout;
        wgpu::BindGroup m_bindGroup;
        wgpu::PipelineLayout m_layout;

        wgpu::Sampler m_sampler;
        wgpu::Texture m_texture;
        wgpu::TextureView m_textureView;

        std::unique_ptr<VertexData> m_vertexBuffer;
        std::unique_ptr<Uniform<MyUniform>> m_uniform;


        wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
        wgpu::TextureView m_depthTextureView;
        wgpu::Texture m_depthTexture;

        uint32_t m_uniformStride = 0;



    };

}
