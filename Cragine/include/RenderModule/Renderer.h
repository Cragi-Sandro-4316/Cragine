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
#include <GLFW/glfw3.h>

namespace crg::renderer {

    size_t constexpr MESH_BUFFER_SIZE = 100000;

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        std::filesystem::path triangle = "assets/triangle.obj";
        std::filesystem::path circle = "assets/circle.obj";
        std::filesystem::path square = "assets/Mesh.obj";

        std::filesystem::path bigMesh = "assets/BigMesh.obj";

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
        transform.scale = vec3(.2);

        transform.rotate(-45, vec3(0, 0, 1));

        auto handle = renderBackend.spawnMesh(bigMesh, material, transform);

        // Transform transform2{};
        // transform2.translation.x = .5;
        // transform2.scale = vec3(1);
        // renderBackend.spawnMesh(bigMesh, material, transform2);

        // renderBackend.unloadMesh(handle);

        Transform transform3{};
        transform3.translation.x = .7;
        transform3.scale = vec3(0.3);
        // transform3.lookAt(vec3(0), vec3(0, 0, 1));
        transform3.rotate(-45, vec3(0, 0, 1));

        renderBackend.spawnMesh(bigMesh, material, transform3);



        Transform transform4{};
        transform4.translation.y = .5;
        transform4.scale = vec3(.5);
        transform4.rotate(-45, vec3(0, 0, 1));

        renderBackend.spawnMesh(bigMesh, material, transform4);



        Transform transform5{};
        transform5.translation.y = -.5;
        transform5.scale = vec3(.5);
        transform5.rotate(-45, vec3(0, 0, 1));
        renderBackend.spawnMesh(bigMesh, material, transform5);


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
        }

        renderPass.end();
        renderPass.release();

        renderContext.queue.submit(cmdEncoder.finish());

        renderContext.surface.present();
    }

}
