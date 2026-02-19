#pragma once

#include "Ecs/Components/QueryResult.h"
#include "Renderer/utils/helpers.h"
#include "Rendererr/Components/MeshPool.h"
#include "Rendererr/Components/ShaderManager.h"
#include "Rendererr/Components/UniformData.h"
#include "Rendererr/RenderContext.h"
#include "Rendererr/utils/Uniform.h"
#include "Rendererr/utils/VertexData.h"
#include "Resources/ResourceParam.h"
#include "utils/Logger.h"
#include <array>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {



    struct FrameData {
        wgpu::SurfaceTexture surfaceTexture;
        wgpu::TextureView targetView;

        wgpu::CommandEncoder encoder;

        struct RenderItem {
            uint32_t vertexCount = 0;
            wgpu::BindGroup bindGroup;
            std::unique_ptr<Uniform<UniformData>> uniformBuffer;
            std::unique_ptr<VertexBuffer> vertexBuffer;
        };

        std::vector<RenderItem> items;

        bool beginFrame(
            wgpu::Device& device,
            wgpu::Surface& surface
        ) {
            surface.getCurrentTexture(&surfaceTexture);

            wgpu::TextureViewDescriptor viewDesc{};
            viewDesc.nextInChain = nullptr;
            viewDesc.label = WGPUStringView("Surface texture view");
            viewDesc.format = wgpuTextureGetFormat(surfaceTexture.texture);
            viewDesc.dimension = WGPUTextureViewDimension_2D;
            viewDesc.baseMipLevel = 0;
            viewDesc.mipLevelCount = 1;
            viewDesc.baseArrayLayer = 0;
            viewDesc.arrayLayerCount = 1;
            viewDesc.aspect = WGPUTextureAspect_All;
            targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);

            if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
                LOG_CORE_ERROR("Surface texture suboptimal");
                return false;
            }

            encoder = device.createCommandEncoder();
            return true;
        }

        void endFrame(wgpu::Queue& queue, wgpu::Surface& surface) {
            wgpu::CommandBuffer commands = encoder.finish();
            queue.submit(commands);

            commands.release();
            encoder.release();
            targetView.release();
            surface.present();
            wgpuTextureRelease(surfaceTexture.texture);
        }

    };



    inline uint32_t ceilToNextMultiple(uint32_t value, uint32_t step) {
        uint32_t divideAndCeil = value / step + (value % step == 0 ? 0 : 1);
        return step * divideAndCeil;
    }

    inline uint32_t getMinAlignment(wgpu::Device& device) {
        wgpu::Limits limits{};
        device.getLimits(&limits);
        return static_cast<uint32_t>(limits.minUniformBufferOffsetAlignment);
    }

    inline void extractData(
        ecs::ResMut<FrameData> frameDataRes,
        ecs::Res<ShaderManager> shaderManagerRes,
        ecs::ResMut<RenderContext> renderContextRes,
        ecs::Res<MeshPool> meshPoolRes
    ) {
        auto& frameData = frameDataRes.get();
        auto& meshPool = meshPoolRes.get();
        auto& shaderManager = shaderManagerRes.get();
        auto& renderContext = renderContextRes.get();

        frameData.items.clear();
        frameData.items.resize(meshPool.meshes().size());

        // Create vertex buffer
        wgpu::BufferDescriptor vertDesc{};
        vertDesc.size = meshPool.meshes().size() * sizeof(VertexAttributes);
        vertDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertDesc.mappedAtCreation = false;

        wgpu::BufferDescriptor uniformDesc{};
        vertDesc.size = ceilToNextMultiple(sizeof(UniformData), getMinAlignment(renderContext.device));
        vertDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        vertDesc.mappedAtCreation = false;

        for (uint32_t i = 0; i < frameData.items.size(); i++) {
            // Init buffers
            // frameData.items[i].vertexBuffer = renderContext.device.createBuffer(vertDesc);

            auto& mesh = meshPool.meshes()[i];

            frameData.items[i].vertexBuffer = std::make_unique<VertexBuffer>(
                renderContext.device,
                renderContext.queue,
                mesh.attributes
            );
            LOG_CORE_INFO("Created vertex buffer");

            frameData.items[i].uniformBuffer = std::make_unique<Uniform<UniformData>>(
                renderContext.device,
                renderContext.queue
            );
            LOG_CORE_INFO("Created uniform buffer");

            frameData.items[i].vertexCount = meshPool.meshes()[i].attributes.size();

            UniformData uniform{};

            frameData.items[i].uniformBuffer->update(uniform);

            // make bind group
            std::array<wgpu::BindGroupEntry, 3> bindings;

            bindings[0].binding = 0;
            bindings[0].buffer = frameData.items[i].uniformBuffer->getBuffer();
            bindings[0].offset = 0;
            bindings[0].size = sizeof(UniformData);

            bindings[1].binding = 1;
           	bindings[1].textureView = shaderManager.getShader(0).textureViews[0];

           	bindings[2].binding = 2;
           	bindings[2].sampler = renderContext.sampler;

            wgpu::BindGroupDescriptor bindGroupDesc;
           	bindGroupDesc.layout = shaderManager.getShader(0).bindGroupLayout;
           	bindGroupDesc.entryCount = (uint32_t)bindings.size();
           	bindGroupDesc.entries = bindings.data();
           	frameData.items[i].bindGroup = renderContext.device.createBindGroup(bindGroupDesc);

        }
    }

    inline void drawFrame(
        ecs::ResMut<FrameData> frameDataRes,
        ecs::ResMut<RenderContext> renderContextRes
    ) {
        auto& renderContext = renderContextRes.get();
        auto& frameData = frameDataRes.get();

        if (!frameData.beginFrame(renderContext.device, renderContext.surface)) {
            return;
        }

        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;

        wgpu::RenderPassColorAttachment renderPassColorAttachment{};
        renderPassColorAttachment.view = frameData.targetView;
        renderPassColorAttachment.resolveTarget = nullptr;

        renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
        renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
        renderPassColorAttachment.clearValue = wgpu::Color{0.1, 0.1, 0.1, 1.0};
        renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment{};

        depthStencilAttachment.view = renderContext.depthView;
        depthStencilAttachment.depthClearValue = 1.0f;
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.depthReadOnly = false;

        depthStencilAttachment.stencilClearValue = 0;
        depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.stencilReadOnly = true;

        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.timestampWrites = nullptr;

        auto renderPass = frameData.encoder.beginRenderPass(renderPassDesc);

        renderPass.setPipeline(renderContext.pipeline);

        for (auto& item : frameData.items) {
            renderPass.setBindGroup(0, item.bindGroup, 0, nullptr);
            renderPass.setVertexBuffer(0, item.vertexBuffer->vertexBuffer(), 0, item.vertexBuffer->vertexBuffer().getSize());
            renderPass.draw(item.vertexCount, 1, 0, 0);
        }

        renderPass.end();
        renderPass.release();

        frameData.endFrame(renderContext.queue, renderContext.surface);
        renderContext.device.poll(false, nullptr);
    }
}
