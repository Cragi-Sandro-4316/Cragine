#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Components/Mesh.h"
#include "RenderModule/Material/Material.h"
#include "RenderModule/RenderBackend.h"
#include "glm/fwd.hpp"
#include <GLFW/glfw3.h>

namespace crg::renderer {


    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        static std::vector<Vertex> verts = {
            Vertex {
                .position = vec4(0., 0.75, 0., 1.),
                .color = vec4(1., 0., 0., 1.)       // This vertex is black
            },
            Vertex {
                .position = vec4(-0.75, -0.75, 0., 1.),
                .color = vec4(0., 1., 0., 1.)       // this vertex is red
            },
            Vertex {
                .position = vec4(0.75, -0.75, 0., 1.),
                .color = vec4(0., 0., 1., 1.)       // this vertex is green
            },
        };

        std::vector<Handle<Buffer>> bufferHandle = { renderBackend.newBuffer<Vertex>(3) };

        Buffer& buffer = renderBackend.getBuffer(bufferHandle[0]);

        buffer.write(
            verts[0],
            0
        );

        buffer.write(
            verts[1],
            1
        );

        buffer.write(
            verts[2],
            2
        );

        renderBackend.newMaterial("../assets/fragVert.wgsl", bufferHandle);

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

            renderPass.draw(material.m_vertexCount , 1, 0, 0);

        }

        renderPass.end();
        renderPass.release();

        renderContext.queue.submit(cmdEncoder.finish());

        renderContext.surface.present();
    }
}
