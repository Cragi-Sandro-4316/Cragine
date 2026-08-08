#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Components/Mesh.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/Texture.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>

namespace crg::renderer {

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        std::filesystem::path meshPath = "../assets/Mesh.obj";

        std::filesystem::path shaderPath = "../assets/fragVert.wgsl";

        std::string texturePath = "../assets/reina.gif";

        Handle<Mesh> meshHandle = renderBackend.loadMesh(meshPath);

        Mesh* mesh = renderBackend.getMeshServer().getMeshPtr(meshHandle);

        Handle<Buffer> vertexBuffer = renderBackend.newBuffer<VertexData>(mesh->vertices.size(), BufferType::Vertex);

        Handle<TextureSampler> sampler = renderBackend.newSampler();

        Handle<Texture> textureHandle = renderBackend.newTexture(texturePath);

        renderBackend.writeBuffer(vertexBuffer, mesh->vertices);

        renderBackend.newMaterial(
            shaderPath,
            mesh->vertices.size(),
            { vertexBuffer },
            { sampler },
            { textureHandle }
        );

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

            renderPass.draw(material.m_totalVertexCount, 1, 0, 0);
        }

        renderPass.end();
        renderPass.release();

        renderContext.queue.submit(cmdEncoder.finish());

        renderContext.surface.present();
    }
}
