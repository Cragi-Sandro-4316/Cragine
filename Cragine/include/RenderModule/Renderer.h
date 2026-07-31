#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Components/Mesh.h"
// #include "RenderModule/Material/Material.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/Texture.h"
#include "glm/fwd.hpp"
#include <GLFW/glfw3.h>

namespace crg::renderer {

    static std::vector<Vertex> verts = {
        Vertex {    // TOP RIGHT
            .position = vec4(0.75, 0.75, 0., 1),
            .color = vec4(1., 0., 0., 1),
        },
        Vertex {    // BOTTOM LEFT
            .position = vec4(-0.75, -0.75, 0., 1),
            .color = vec4(0., 1., 0., 1),
        },
        Vertex {    // BOTTOM RIGHT
            .position = vec4(0.75, -0.75, 0., 1),
            .color = vec4(0., 0., 1., 1),
        },
        Vertex {    // TOP LEFT
            .position = vec4(-0.75, 0.75, 0., 1),
            .color = vec4(1., 0., 0., 1),
        },
    };

    static std::vector<Index> idxs = {
        {0}, {1}, {2}, {3}, {1}, {0}
    };

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        std::vector<Handle<Buffer>> bufferHandles = {
            renderBackend.newBuffer<Vertex>(4, BufferType::Storage),
            renderBackend.newBuffer<Index>(6, BufferType::Storage)
        };

        std::vector<Handle<Texture>> textureHandles = { renderBackend.newTexture() };

        Buffer& vertexBuffer = renderBackend.getBuffer(bufferHandles[0]);
        Buffer& indexBuffer = renderBackend.getBuffer(bufferHandles[1]);

        vertexBuffer.writeBuffer(verts);
        indexBuffer.writeBuffer(idxs);

        renderBackend.newMaterial("../assets/fragVert.wgsl", &bufferHandles, &textureHandles);

        LOG_CORE_INFO("Material created");
    }

    static void render(
        ResMut<RenderBackend> rRenderBackend
    ) {
        auto& renderContext = rRenderBackend.get().getRenderContext();
        auto& materialCache = rRenderBackend.get().getMaterialCache();

        wgpu::SurfaceTexture drawable;
        renderContext.surface.getCurrentTexture(&drawable);

        wgpu::TextureViewDescriptor imgViewDesc{};
        imgViewDesc.label = wgpu::StringView("Surface texture view");
        imgViewDesc.format = renderContext.surfaceFormat;
        imgViewDesc.dimension = WGPUTextureViewDimension_2D;
        imgViewDesc.baseMipLevel = 0;
        imgViewDesc.mipLevelCount = 1;
        imgViewDesc.baseArrayLayer = 0;
        imgViewDesc.arrayLayerCount = 1;
        imgViewDesc.aspect = WGPUTextureAspect_All;
        wgpu::TextureView imgView = wgpuTextureCreateView(drawable.texture, &imgViewDesc);

        wgpu::CommandEncoderDescriptor cmdEncoderDesc{};
        cmdEncoderDesc.nextInChain = nullptr;
        wgpu::CommandEncoder cmdEncoder = renderContext.device.createCommandEncoder(cmdEncoderDesc);

        std::vector<wgpu::RenderPassColorAttachment> colorAttachments;
        colorAttachments.emplace_back();
        colorAttachments[0].view = imgView;
        colorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        colorAttachments[0].clearValue = wgpu::Color(0.3, 0.3, 0.3, 0.0);
        colorAttachments[0].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;
        renderPassDesc.colorAttachmentCount = colorAttachments.size();
        renderPassDesc.colorAttachments = colorAttachments.data();


        wgpu::RenderPassEncoder renderPass = cmdEncoder.beginRenderPass(renderPassDesc);


        for (auto& material : materialCache.getMaterials()) {
            renderPass.setPipeline(material.m_pipeline);

            renderPass.setBindGroup(0, material.m_binding, 0, nullptr);

            renderPass.draw(material.m_indexCount, 1, 0, 0);

        }

        renderPass.end();
        renderPass.release();

        renderContext.queue.submit(cmdEncoder.finish());

        renderContext.surface.present();
    }
}
