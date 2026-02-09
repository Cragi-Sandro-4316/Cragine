#pragma once

#include "Ecs/Components/QueryResult.h"
#include "Rendererr/Components/MeshPool.h"
#include "Rendererr/RenderContext.h"
#include "Resources/ResourceParam.h"
#include "utils/Logger.h"
#include <glm/ext/vector_float3.hpp>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    struct MeshHandle;
    struct MeshPool;

    struct FrameData {
        wgpu::SurfaceTexture surfaceTexture;
        wgpu::TextureView targetView;

        wgpu::CommandEncoder encoder;

        struct RenderItem {
            wgpu::Buffer vertexBuffer;
            uint32_t vertexCount = 0;
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

    inline void extractData(
        ecs::ResMut<FrameData> frameDataRes,
        ecs::ResMut<RenderContext> renderContextRes,
        ecs::Res<MeshPool> meshPoolRes
    ) {
        auto& frameData = frameDataRes.get();
        auto& meshPool = meshPoolRes.get();
        auto& renderContext = renderContextRes.get();

        frameData.items.clear();
        frameData.items.resize(meshPool.meshes().size());

        // Create vertex buffer
        wgpu::BufferDescriptor vertDesc{};
        vertDesc.size = meshPool.meshes().size() * sizeof(VertexAttributes);
        vertDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertDesc.mappedAtCreation = false;


        for (uint32_t i = 0; i < frameData.items.size(); i++) {
            frameData.items[i].vertexBuffer = renderContext.device.createBuffer(vertDesc);

            renderContext.queue.writeBuffer(
                frameData.items[i].vertexBuffer,
                0,
                &meshPool.meshes()[i].attributes,
                meshPool.meshes().size() * sizeof(VertexAttributes)
            );
            frameData.items[i].vertexCount = meshPool.meshes()[i].attributes.size();
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

        // for (auto& item : frameData.items) {
        //     renderPass.setVertexBuffer(0, item.vertexBuffer, 0, item.vertexBuffer.getSize());
        //     renderPass.draw(item.vertexCount, 1, 0, 0);
        // }

        renderPass.end();
        renderPass.release();

        frameData.endFrame(renderContext.queue, renderContext.surface);
        renderContext.device.poll(false, nullptr);
    }
}
