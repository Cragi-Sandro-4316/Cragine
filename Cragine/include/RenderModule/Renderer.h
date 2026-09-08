#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Components/Camera.h"
#include "RenderModule/Handles.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/ImageTexture.h"
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

        std::filesystem::path triangle = "assets/triangle.obj";
        std::filesystem::path circle = "assets/circle.obj";
        std::filesystem::path square = "assets/Mesh.obj";
        std::filesystem::path cube = "assets/cube.obj";
        std::filesystem::path plane = "assets/plane.obj";

        std::filesystem::path bigMesh = "assets/BigMesh.obj";

        std::filesystem::path shaderPath = "assets/fragVert.wgsl";

        std::filesystem::path texturePath = "assets/immo.png";

        Handle<TextureSampler> sampler = renderBackend.newSampler();

        Handle<ImageTexture> textureHandle = renderBackend.newTexture(texturePath);

        Handle<Buffer> debugBuffer = renderBackend.newBuffer<float32_t>(30, StorageReadable);

        std::vector<float32_t>vec(30);
        renderBackend.writeBuffer(debugBuffer, vec);

        Camera camera{};
        camera.setPerspectiveProjection(
            50,
            1,
            0.1,
            10
        );


        auto atlasHandle = renderBackend.newAtlas(16);

        // atlasHandle.pushTexture(
        //     texturePath,
        //     renderBackend.getRenderContext().queue,
        //     0
        // );

        Handle<Material> material = renderBackend.newMaterial(
            shaderPath,
            camera,
            MeshBufferSize::Large,
            debugBuffer,
            sampler,
            atlasHandle
        );

        Transform transform{};
        transform.translation.z = 2;
        transform.scale = vec3(.5);
        transform.rotate(-90, vec3(1, 0, 0));

        renderBackend.spawnMesh(plane, material, transform);
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
