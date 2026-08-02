#pragma once
#include "Ecs/Ecs.h"
#include "RenderModule/Components/Mesh.h"
// #include "RenderModule/Material/Material.h"
#include "RenderModule/RenderBackend.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/Texture.h"
#include "glm/fwd.hpp"
#include <GLFW/glfw3.h>

namespace crg::renderer {

    static std::vector<Vertex> verts = {
        Vertex {    // TOP RIGHT
            .position = vec3(0.75, 0.75, 0.),
            // .color = vec3(1., 0., 0.),
            .uv = vec2(0, 256)
        },
        Vertex {    // BOTTOM LEFT
            .position = vec3(-0.75, -0.75, 0.),
            // .color = vec3(0., 1., 0.),
            .uv = vec2(-55, 0)
        },
        Vertex {    // BOTTOM RIGHT
            .position = vec3(0.75, -0.75, 0.),
            // .color = vec3(0., 0., 1.),
            .uv = vec2(-55, 256)
        },
        Vertex {    // TOP LEFT
            .position = vec3(-0.75, 0.75, 0.),
            // .color = vec3(1., 0., 0.),
            .uv = vec2(0, 0)
        },
        Vertex {    // BOTTOM LEFT
            .position = vec3(-0.75, -0.75, 0.),
            // .color = vec3(0., 1., 0.),
            .uv = vec2(-55, 0)
        },
        Vertex {    // TOP RIGHT
            .position = vec3(0.75, 0.75, 0.),
            // .color = vec3(1., 0., 0.),
            .uv = vec2(0, 256)
        },
    };

    static std::vector<Index> idxs = {
        {0}, {1}, {2}, {3}, {1}, {0}
    };

    static void newMaterial(
        ResMut<RenderBackend> rGpuHandler
    ) {
        auto& renderBackend = rGpuHandler.get();

        std::filesystem::path meshPath = "../assets/Mesh.obj";

        std::filesystem::path shaderPath = "../assets/fragVert.wgsl";

        Handle<Mesh> meshHandle = renderBackend.loadMesh(meshPath);

        Mesh* mesh = renderBackend.getMeshServer().getMeshPtr(meshHandle);

        LOG_CORE_INFO("Mesh size: {}", mesh->vertices.size());
        Handle<Buffer> vertexBuffer = renderBackend.newBuffer<Vertex>(mesh->vertices.size(), BufferType::Storage);

        // Handle<Buffer> indexBuffer = renderBackend.newBuffer<Index>(6, BufferType::Storage);

        Handle<TextureSampler> sampler = renderBackend.newSampler();

        Handle<Texture> textureHandle = renderBackend.newTexture();

        renderBackend.writeBuffer(vertexBuffer, mesh->vertices);
        // renderBackend.writeBuffer(indexBuffer, idxs);

        renderBackend.newMaterial(
            shaderPath,
            mesh->vertices.size(),
            {
                vertexBuffer,
                // indexBuffer
            },
            {sampler},
            {textureHandle}
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

            renderPass.draw(material.m_indexCount, 1, 0, 0);

        }

        renderPass.end();
        renderPass.release();

        renderContext.queue.submit(cmdEncoder.finish());

        renderContext.surface.present();
    }
}
