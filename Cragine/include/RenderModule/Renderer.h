#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Material/Material.h"
#include "RenderModule/RenderBackend.h"
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.hpp>


namespace crg::renderer {


    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        renderBackend.newMaterial();
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
        colorAttachments[0].clearValue = wgpu::Color(1., 1., 0., 0.0);
        colorAttachments[0].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;
        renderPassDesc.colorAttachmentCount = colorAttachments.size();
        renderPassDesc.colorAttachments = colorAttachments.data();


        wgpu::RenderPassEncoder renderPass = cmdEncoder.beginRenderPass(renderPassDesc);


        const Material& material = materialCache.getMaterial(0);
        renderPass.setPipeline(material.m_pipeline);

        renderPass.setVertexBuffer(0, material.m_buffer, 0, material.m_buffer.getSize());

        renderPass.draw(material.m_vertexCount, 1, 0, 0);

        renderPass.end();
        renderPass.release();

        renderContext.queue.submit(cmdEncoder.finish());

        renderContext.surface.present();
    }
}
