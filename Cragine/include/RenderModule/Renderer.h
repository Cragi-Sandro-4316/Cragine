#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Components/Camera.h"
#include "RenderModule/Handles.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Components/Transform.h"
#include "glm/fwd.hpp"
#include <GLFW/glfw3.h>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    size_t constexpr MESH_BUFFER_SIZE = 100000;

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        Handle<TextureSampler> sampler = renderBackend.newSampler();


        Camera camera{};
        camera.setPerspectiveProjection(
            50,
            1,
            0.1,
            10
        );

        auto atlasHandle = renderBackend.newAtlas(16);
        Handle<AtlasEntry> boredHandle = renderBackend.writeAtlas(atlasHandle, "../assets/emilia.png");
        Handle<AtlasEntry> immoHandle = renderBackend.writeAtlas(atlasHandle, "../assets/immo.png");
        Handle<AtlasEntry> reinaHandle = renderBackend.writeAtlas(atlasHandle, "../assets/yukari.png");

        Handle<Material> material = renderBackend.newMaterial(
            "../assets/fragVert.wgsl",
            camera,
            MeshBufferSize::Large,
            sampler,
            atlasHandle
        );

        Transform transform{};
        transform.translation.z = 2;
        transform.translation.x = -0.4;
        transform.scale = vec3(.25);
        transform.rotate(-45, vec3(0, 1, 0));
        transform.rotate(-20, vec3(1, 0, 0));

        renderBackend.spawnMesh(
            "../assets/cube.obj",
            material,
            reinaHandle,
            transform
        );



        Transform transform2{};
        transform2.translation.z = 2;
        transform2.translation.x = 0.4;
        transform2.translation.y = 0.4;
        transform2.scale = vec3(.3);
        transform2.rotate(-20, vec3(1, 0, 0));

        renderBackend.spawnMesh(
            "../assets/sphere.obj",
            material,
            boredHandle,
            transform2
        );



        Transform transform3{};
        transform3.translation.z = 2;
        transform3.translation.x = 0.4;
        transform3.translation.y = -0.4;
        transform3.scale = vec3(.3);
        transform3.rotate(-20, vec3(1, 0, 0));

        renderBackend.spawnMesh(
            "../assets/pyramid.obj",
            material,
            immoHandle,
            transform3
        );

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

    }



}
