#pragma once

#include "Ecs/Commands/Command.h"
#include "Ecs/Commands/Commands.h"
#include "Ecs/Ecs.h"
#include "Ecs/Entity/Entity.h"
#include "Ecs/SystemParams/QueryParam.h"
#include "Ecs/SystemParams/ResParam.h"
#include "RenderModule/Components/Camera.h"
#include "RenderModule/Components/Transform.h"
#include "RenderModule/GpuInterface.h"
#include "RenderModule/GpuResourceManager.h"
#include "RenderModule/Managers/MaterialManager.h"
#include "RenderModule/Managers/BindGroupManager.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/CameraBuffer.h"
#include "RenderModule/Structs/GpuResource.h"
#include "RenderModule/Structs/TextureAtlas.h"

namespace crg::renderer {


    static void setupCameras(
        Query<Entity, Camera> q,
        Commands commands,
        ResMut<MaterialManager> rMaterials,
        ResMut<GpuInterface> rGpuInterface
    ) {
        MaterialManager& materials = rMaterials.get();
        GpuInterface& gpuInterface = rGpuInterface.get();

        for (auto [entity, camera] : q) {
            Handle<GpuResource> handle = gpuInterface.newCamera(
                camera,
                entity
            );

            commands.addComponent(entity, handle);
        }
    }


    struct Sample {
        MeshCollection& meshCollection;
        CameraBuffer& cameraBuffer;
        Sampler& sampler;
        TextureAtlas& atlas;
    };


    static void spawnExample(
        Commands commands,
        ResMut<MaterialManager> rMaterials,
        ResMut<GpuInterface> rGpuInterface
    ) {
        MaterialManager& materials = rMaterials.get();
        GpuInterface& gpuInterface = rGpuInterface.get();

        Camera cam{};
        cam.setPerspectiveProjection(
            50,
            1,
            0.1,
            10
        );

        auto cameraEntity = commands.spawn(cam);

        LOG_CORE_INFO("ent: {}", cameraEntity.id);

        auto meshes = gpuInterface.newMeshCollection(MeshCollection::Size::Large);
        auto camera = gpuInterface.newCamera(cam, cameraEntity);
        auto samp = gpuInterface.newSampler();
        auto atlas = gpuInterface.newAtlas(8);

        materials.newMaterial(
            "../assets/fragVert.wgsl",
            gpuInterface.getRenderContext(),
            Sample {
                gpuInterface.getMeshCollection(meshes),
                gpuInterface.getCamera(camera),
                gpuInterface.getSampler(samp),
                gpuInterface.getAtlas(atlas)
            }
        );

        auto textureHandle = gpuInterface.pushTexture(atlas, "../assets/emilia.png");

        Transform transform{};
        transform.translation.z = 2;
        transform.translation.x = -0.4;
        transform.scale = vec3(.25);
        transform.rotate(-45, vec3(0, 1, 0));
        transform.rotate(-20, vec3(1, 0, 0));

        std::filesystem::path meshPath = "../assets/cube.obj";

        gpuInterface.getMeshCollection(meshes).loadMesh(
            meshPath,
            transform,
            textureHandle
        );



        // auto atlas = gpuInterface.newAtlas(8);
        // auto texture = gpuInterface.pushTexture(atlas, "../assets/emilia.png");

        // commands.spawn(
        //     Mesh {
        //         .path = "../assets/sphere.obj",
        //         .material = materials.newMaterial(
        //             "../assets/fragVert.wgsl",
        //             {
        //                 gpuInterface.newMeshCollection(MeshCollection::Size::Large),
        //                 gpuInterface.newCamera(cam, cameraEntity),
        //                 gpuInterface.newSampler(),
        //                 gpuInterface.newAtlas(8)
        //             },
        //             gpuInterface,
        //             materialUpdate
        //         ),
        //         .texture = texture
        //     },
        //     Transform{}
        // );
    }


    static void spawnMeshes(
        Query<Entity, Mesh, Transform> q,
        Commands commands,
        ResMut<MaterialManager> rMaterials,
        ResMut<GpuInterface> rGpuInterface
    ) {
        // MaterialManager& materials = rMaterials.get();
        // GpuInterface& gpuInterface = rGpuInterface.get();


        // for (auto [entity, mesh, transform] : q) {
        //     Material& material = materials.getMaterial(mesh.material);

        //     for (auto& handle : material.m_resources) {

        //         auto type = gpuInterface.getHandleType(handle);

        //         if (type == GpuResource::Type::MeshCollection) {
        //             auto meshHandle = gpuInterface.getMeshCollection(handle).loadMesh(
        //                 mesh.path,
        //                 transform,
        //                 mesh.texture
        //             );

        //             commands.removeComponent<Mesh>(entity);
        //             commands.addComponent(entity, meshHandle);
        //             break;
        //         }

        //     }
    }




    static void setup(
        Query<Entity, Handle<GpuResource>, With<Camera>> q,
        ResMut<MaterialManager> rMaterials,
        ResMut<GpuInterface> rGpuInterface
    ) {
        // MaterialManager& materials = rMaterials.get();
        // GpuInterface& gpuInterface = rGpuInterface.get();

        // std::filesystem::path shaderPath = "../assets/fragVert.wgsl";

        // Handle<GpuResource> meshCollection = gpuInterface.newMeshCollection(MeshCollection::Size::Large);
        // Handle<GpuResource> atlas = gpuInterface.newAtlas(8);

        // Handle<GpuResource> camera{};
        // for (auto [entity, gpuRes] : q) {
        //     camera = gpuRes;
        // }

        // auto materialHandle = materials.newMaterial(
        //     shaderPath,
        //     {
        //         meshCollection,
        //         camera,
        //         gpuInterface.newSampler(),
        //         atlas
        //     },
        //     gpuInterface,
        //     materialUpdate
        // );

        // auto texture = gpuInterface.pushTexture(atlas, "../assets/emilia.png");

        // Transform transform{};
        // transform.translation.z = 2;
        // transform.translation.x = -0.4;
        // transform.scale = vec3(.25);
        // transform.rotate(-45, vec3(0, 1, 0));
        // transform.rotate(-20, vec3(1, 0, 0));

        // gpuInterface.getMeshCollection(meshCollection).loadMesh(
        //     "../assets/pyramid.obj",
        //     transform,
        //     texture
        // );
    }

    static void runMaterialUpdates(
        ResMut<GpuInterface> rGpuResManager,
        ResMut<MaterialManager> rMaterialManager

    ) {
        // MaterialManager& materials = rMaterialManager.get();

        // materials.runUpdates(rGpuResManager.get().getResourceManager());
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

        for (auto& [type, material] : materialCache.getMaterials()) {
            renderPass.setPipeline(material->m_pipeline);

            renderPass.setBindGroup(0, material->m_bindGroup, 0, nullptr);

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
