#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Handles.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/Texture.h"
#include "RenderModule/Transform.h"
#include "glm/fwd.hpp"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>

namespace crg::renderer {

    size_t constexpr MESH_BUFFER_SIZE = 100000;

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        std::filesystem::path triangle = "assets/triangle.obj";
        std::filesystem::path circle = "assets/circle.obj";

        std::filesystem::path shaderPath = "assets/fragVert.wgsl";

        std::string texturePath = "assets/reina.gif";

        Handle<TextureSampler> sampler = renderBackend.newSampler();

        Handle<Texture> textureHandle = renderBackend.newTexture(texturePath);

        Handle<Buffer> debugBuffer = renderBackend.newBuffer<float32_t>(30, StorageReadable);

        std::vector<float32_t>vec(30);
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


        Transform transform2{};
        transform2.translation.x = .5;
        transform2.scale = vec3(1);

        renderBackend.spawnMesh(triangle, material, transform);
        renderBackend.spawnMesh(circle, material, transform2);

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
            std::vector<MeshChunk> buff{};

            material.m_meshBuffer.chunkBuffer().read(buff);

            MeshChunk& chunk1 = buff[0];
            MeshChunk& chunk2 = buff[1];

            LOG_CORE_INFO("Object 1 vertex pos: ({}, {}, {})", chunk1.vertexData[0].position.x, chunk1.vertexData[0].position.y, chunk1.vertexData[0].position.z);
            LOG_CORE_INFO("Object 2 vertex pos: ({}, {}, {})", chunk2.vertexData[0].position.x, chunk2.vertexData[0].position.y, chunk2.vertexData[0].position.z);

            // LOG_CORE_INFO("Object 1 Map index {}", buff[0]);
            // LOG_CORE_INFO("Object 1 translation: ({}, {}, {})", buff[1], buff[2], buff[3]);
            // LOG_CORE_INFO("Object 1 scale: ({}, {}, {})", buff[8], buff[9], buff[10]);



            // LOG_CORE_INFO("Object 2 Map index {}", buff[4]);
            // LOG_CORE_INFO("Object 2 transform: ({}, {}, {})", buff[5], buff[6], buff[7]);
            // LOG_CORE_INFO("Object 2 scale: ({}, {}, {})", buff[11], buff[12], buff[13]);
            // LOG_CORE_INFO("Object 2 vertex pos: ({}, {}, {})", buff[17], buff[18], buff[19]);


            // for (int i = 0; i < 30; i++) {
            //     LOG_CORE_INFO("debug buffer {}: {}", i, buff[i]);
            // }


        }
    }
}
