#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Components/Camera.h"
#include "RenderModule/Handles.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Components/Transform.h"
#include "glm/fwd.hpp"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    size_t constexpr MESH_BUFFER_SIZE = 100000;

    Handle<Buffer> debugBuffer{};

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        Handle<TextureSampler> sampler = renderBackend.newSampler();

        debugBuffer = renderBackend.newBuffer<float32_t>(10, BufferType::Debug);

        Camera camera{};
        camera.setPerspectiveProjection(
            50,
            1,
            0.1,
            10
        );

        auto atlasHandle = renderBackend.newAtlas(8);
        renderBackend.writeAtlas(atlasHandle, "../assets/chunk_testing.png");
        renderBackend.writeAtlas(atlasHandle, "../assets/reina.gif");
        renderBackend.writeAtlas(atlasHandle, "../assets/immo.png");
        renderBackend.writeAtlas(atlasHandle, "../assets/reina.gif");
        renderBackend.writeAtlas(atlasHandle, "../assets/plunder.png");
        renderBackend.writeAtlas(atlasHandle, "../assets/immo.png");


        Handle<Material> material = renderBackend.newMaterial(
            "../assets/fragVert.wgsl",
            camera,
            MeshBufferSize::Large,
            sampler,
            atlasHandle,
            debugBuffer
        );

        Transform transform{};
        transform.translation.z = 2;
        transform.scale = vec3(.5);
        transform.rotate(-90, vec3(1, 0, 0));

        renderBackend.spawnMesh("../assets/plane.obj", material, transform);
    }


    static void render(
        ResMut<RenderBackend> rRenderBackend
    ) {
        auto& renderContext = rRenderBackend.get().getRenderContext();
        auto& materialCache = rRenderBackend.get().getMaterialCache();

        wgpu::SurfaceTexture surfaceTex;
        renderContext.surface.getCurrentTexture(&surfaceTex);

        wgpu::TextureViewDescriptor surfaceTexViewDesc{};
        surfaceTexViewDesc.label = wgpu::StringView("Surface texture view");
        surfaceTexViewDesc.format = renderContext.surfaceFormat;
        surfaceTexViewDesc.dimension = WGPUTextureViewDimension_2D;
        surfaceTexViewDesc.baseMipLevel = 0;
        surfaceTexViewDesc.mipLevelCount = 1;
        surfaceTexViewDesc.baseArrayLayer = 0;
        surfaceTexViewDesc.arrayLayerCount = 1;
        surfaceTexViewDesc.aspect = WGPUTextureAspect_All;
        wgpu::TextureView surfaceTexView = wgpuTextureCreateView(surfaceTex.texture, &surfaceTexViewDesc);

        wgpu::CommandEncoderDescriptor cmdEncoderDesc{};
        cmdEncoderDesc.nextInChain = nullptr;
        wgpu::CommandEncoder cmdEncoder = renderContext.device.createCommandEncoder(cmdEncoderDesc);

        std::vector<wgpu::RenderPassColorAttachment> colorAttachments;
        colorAttachments.emplace_back();
        colorAttachments[0].view = surfaceTexView;
        colorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        colorAttachments[0].clearValue = wgpu::Color(0.3, 0.3, 0.3, 0.0);
        colorAttachments[0].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;
        renderPassDesc.colorAttachmentCount = colorAttachments.size();
        renderPassDesc.colorAttachments = colorAttachments.data();
        renderPassDesc.depthStencilAttachment = &renderContext.depthStencilAttachment;

        wgpu::RenderPassEncoder renderPass = cmdEncoder.beginRenderPass(renderPassDesc);

        for (auto& material : materialCache.getMaterials()) {
            renderPass.setPipeline(material.m_pipeline);

            renderPass.setBindGroup(0, material.m_binding, 0, nullptr);

            renderPass.draw(material.m_meshBuffer.vertexCount(), 1, 0, 0);
        }

        renderPass.end();
        renderPass.release();

        auto commandBuffer = cmdEncoder.finish();
        renderContext.queue.submit(commandBuffer);

        commandBuffer.release();

        renderContext.surface.present();

        surfaceTexView.release();
        wgpuTextureRelease(surfaceTex.texture);
        cmdEncoder.release();

        auto bufferView = rRenderBackend.get().getBuffer(debugBuffer).getBufferView<float32_t>();

        LOG_CORE_ERROR("DebugBuffer:");
        for (int i = 0; i < 10; i++) {
            LOG_CORE_INFO("DebugBuffer[{}]: {}", i, bufferView[i]);
        }

    }



}
