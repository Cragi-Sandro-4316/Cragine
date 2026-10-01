#pragma once
#include <webgpu/webgpu.hpp>

#include "Ecs/Ecs.h"

#include "RenderModule/Components/Camera.h"
#include "RenderModule/Managers/MaterialManager.h"
#include "RenderModule/Managers/GpuResource.h"
#include "RenderModule/Managers/BufferResource.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/ImageTexture.h"
#include "RenderModule/Structs/MaterialParam.h"
#include "RenderModule/Structs/TextureAtlas.h"

namespace crg::renderer {

    struct SampleMaterial {

        MaterialParam<
            wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex,
            Buffer<wgpu::BufferBindingType::Uniform, CameraUniform>
        > camera;

        MaterialParam<
            wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex,
            Sampler
        > sampler;

        MaterialParam<
            wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex,
            TextureAtlas
        > atlas;
    };


    static void spawnExample(
        Commands commands,
        ResMut<RenderContext> rRenderContext,
        ResMut<MaterialManager> rMaterials,
        ResMut<GpuResource<IBuffer>> rBufferManager,
        ResMut<GpuResource<Sampler>> rSamplerManager,
        ResMut<GpuResource<TextureAtlas>> rTextureAtlasManager
    ) {
        RenderContext& renderContext = rRenderContext.get();
        MaterialManager& materials = rMaterials.get();
        auto& bufferManager =  rBufferManager.get();
        auto& samplerManager =  rSamplerManager.get();
        auto& textureAtlasManager =  rTextureAtlasManager.get();

        Camera cam{};
        cam.setPerspectiveProjection(
            50,
            1,
            0.1,
            10
        );

        auto cameraEntity = commands.spawn(cam);

        LOG_CORE_INFO("ent: {}", cameraEntity.id);

        std::filesystem::path p = "../assets/fragVert.wgsl";


        auto camera = bufferManager.add<wgpu::BufferBindingType::Uniform, CameraUniform>(
            renderContext,
            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform
        );
        auto sampler = samplerManager.add(renderContext);
        auto atlas = textureAtlasManager.add(renderContext);


        auto mat = materials.newMaterial(
            p,
            renderContext,
            SampleMaterial {
                .camera = { camera },
                .sampler = { sampler },
                .atlas = { atlas }
            }
        );

        auto mesh = mat.m_meshCollection.loadMesh(
            "../assets/cube.obj",
            textureAtlasManager.get(atlas).pushTexture(
                "../assets/emilia.gif",
                renderContext.queue,
                1
            )
        );

        Transform transform{};
        transform.translation.z = 2;
        transform.translation.x = -0.4;
        transform.scale = vec3(.25);
        transform.rotate(-45, vec3(0, 1, 0));
        transform.rotate(-20, vec3(1, 0, 0));

        mat.m_meshCollection.addInstance(
            transform,
            mesh
        );

        Camera cameraObj{};
        cameraObj.setPerspectiveProjection(
            50,
            1,
            0.1,
            10
        );

        bufferManager.get(camera).write(cameraObj.getUniform());
    }



    static void render(
        ResMut<RenderContext> rRenderContext,
        ResMut<MaterialManager> rMaterialManager,
        ResMut<GpuResource<IBuffer>> rBufferManager,
        ResMut<GpuResource<ImageTexture>> rImageTextureManager,
        ResMut<GpuResource<Sampler>> rSamplerManager,
        ResMut<GpuResource<TextureAtlas>> rTextureAtlasManager
    ) {
        auto& materialCache = rMaterialManager.get();
        auto& renderContext = rRenderContext.get();
        auto& bufferManager =  rBufferManager.get();
        auto& imageTextureManager =  rImageTextureManager.get();
        auto& samplerManager =  rSamplerManager.get();
        auto& textureAtlasManager =  rTextureAtlasManager.get();


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

        for (auto& [type, material] : materialCache.getMaterials()) {
            renderPass.setPipeline(material->m_pipeline);

            renderPass.setBindGroup(
                0,
                material->getBindGroup(
                    renderContext,
                    bufferManager,
                    imageTextureManager,
                    samplerManager,
                    textureAtlasManager
                ),
                0,
                nullptr
            );

            renderPass.draw(material->getVertexCount(), 1, 0, 0);
            LOG_CORE_INFO("material vert count: {}", material->getVertexCount());
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
