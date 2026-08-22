#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Handles.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/Texture.h"
#include "RenderModule/Transform.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <cstdint>

namespace crg::renderer {

    size_t constexpr MESH_BUFFER_SIZE = 100000;

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        std::filesystem::path meshPath = "assets/Mesh.obj";
        std::filesystem::path meshPath2 = "assets/Mesh2.obj";
        std::filesystem::path meshPath3 = "assets/Mesh3.obj";

        std::filesystem::path shaderPath = "assets/fragVert.wgsl";

        std::string texturePath = "assets/reina.gif";

        Handle<TextureSampler> sampler = renderBackend.newSampler();

        Handle<Texture> textureHandle = renderBackend.newTexture(texturePath);

        Handle<Buffer> debugBuffer = renderBackend.newBuffer<uint32_t>(10, StorageReadable);

        std::vector<uint32_t>vec(10);
        renderBackend.writeBuffer(debugBuffer, vec);

        Handle<Material> material = renderBackend.newMaterial(
            shaderPath,
            MeshBufferSize::Large,
            debugBuffer,
            sampler,
            textureHandle
        );

        Transform transform{};
        transform.translation.x = -0.5;
        transform.scale = vec3(.5);

        Handle<Mesh> meshHandle = renderBackend.spawnMesh(meshPath, material, transform);

        Transform transform2{};
        transform2.translation.x = .5;
        transform2.scale = vec3(1);
        // renderBackend.spawnMesh(meshPath, material, transform2);

        renderBackend.spawnMesh(meshPath2, material, transform2);

        // renderBackend.spawnMesh(meshPath3, material, Transform{});

        // renderBackend.unloadMesh(meshHandle);

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

            renderPass.draw(material.m_meshBuffer.vertexCount(), 1, 0, 0);

            LOG_CORE_TRACE("Material rendered");
        }

        renderPass.end();
        renderPass.release();

        renderContext.queue.submit(cmdEncoder.finish());

        renderContext.surface.present();
    }

    static void logBufferContents(
        ResMut<RenderBackend> rRenderBackend
    ) {
        auto& materialCache = rRenderBackend.get().getMaterialCache();

        for (auto& material : materialCache.getMaterials()) {
            std::vector<uint32_t> buff{};

            material.m_buffers[0].read(buff);

            LOG_CORE_INFO("Object 1 Map index {}", buff[0]);
            LOG_CORE_INFO("Object 1 Chunk index {}", buff[1]);
            LOG_CORE_INFO("Object 1 Instance index {}", buff[2]);


            LOG_CORE_INFO("Object 2 Map index {}", buff[4]);
            LOG_CORE_INFO("Object 2 Chunk index {}", buff[5]);
            LOG_CORE_INFO("Object 2 Instance index {}", buff[6]);


        }
    }
}
