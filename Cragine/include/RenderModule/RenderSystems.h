#pragma once

#include "Ecs/Ecs.h"
#include "RenderModule/Components/Camera.h"
#include "RenderModule/GpuInterface.h"
#include "RenderModule/GpuResourceManager.h"
#include "RenderModule/Managers/MaterialManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Managers/BindGroupManager.h"
#include "RenderModule/Structs/GpuResource.h"

namespace crg::renderer {


    static void materialUpdate(
        Material& material,
        GpuResourceManager& resManager
    ) {

        for(auto& handle : material.m_resources) {
            auto type = resManager.getType(handle);
            if (type == GpuResource::Type::MeshCollection) {

                MeshCollection& meshes = resManager.getMeshCollection(handle);

                material.m_vertexCount = meshes.vertexCount();

                break;
            }
        }


    }


    static void setup(
        ResMut<MaterialManager> rMaterials,
        ResMut<GpuInterface> rGpuInterface
    ) {
        MaterialManager& materials = rMaterials.get();
        GpuInterface& gpuInterface = rGpuInterface.get();

        std::filesystem::path shaderPath = "../assets/fragVert.wgsl";

        Handle<GpuResource> cameraBuffer = gpuInterface.newBuffer<CameraUniform>(1, Uniform);
        Handle<GpuResource> meshCollection = gpuInterface.newMeshCollection(MeshCollection::Size::Large);
        Handle<GpuResource> atlas = gpuInterface.newAtlas(8);

        auto materialHandle = materials.newMaterial(
            shaderPath,
            {
                meshCollection,
                cameraBuffer,
                gpuInterface.newSampler(),
                atlas
            },
            gpuInterface,
            materialUpdate
        );


        Camera camera{};
        camera.setPerspectiveProjection(
            50,
            1,
            0.1,
            10
        );

        gpuInterface.getBuffer(cameraBuffer).write(camera.getUniform());

        auto texture = gpuInterface.pushTexture(atlas, "../assets/emilia.png");

        Transform transform{};
        transform.translation.z = 2;
        transform.translation.x = -0.4;
        transform.scale = vec3(.25);
        transform.rotate(-45, vec3(0, 1, 0));
        transform.rotate(-20, vec3(1, 0, 0));

        gpuInterface.getMeshCollection(meshCollection).loadMesh(
            "../assets/pyramid.obj",
            transform,
            texture
        );
    }

    static void runMaterialUpdates(
        ResMut<GpuInterface> rGpuResManager,
        ResMut<MaterialManager> rMaterialManager

    ) {
        MaterialManager& materials = rMaterialManager.get();

        materials.runUpdates(rGpuResManager.get().getResourceManager());
    }


    static void render(
        ResMut<GpuInterface> rGpuInterface,
        ResMut<MaterialManager> rMaterialManager,
        ResMut<BindGroupManager> rBindgroupManager
    ) {
        auto& gpuInterface = rGpuInterface.get();
        auto& materialCache = rMaterialManager.get();
        auto& bindGroupManager = rBindgroupManager.get();

        auto& renderContext = gpuInterface.getRenderContext();

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

        for (auto& [materialID, material] : materialCache.getMaterials()) {
            renderPass.setPipeline(material.m_pipeline);

            wgpu::BindGroup& binding = bindGroupManager.getBindGroup(gpuInterface, materialID, material);

            renderPass.setBindGroup(0, binding, 0, nullptr);

            renderPass.draw(material.m_vertexCount, 1, 0, 0);
            LOG_CORE_INFO("material vert count: {}", material.m_vertexCount);
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
