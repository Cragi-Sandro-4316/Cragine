#pragma once

#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/Material.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/ImageTexture.h"
#include "RenderModule/Handles.h"

#include <fstream>
#include <vector>
#include <webgpu/webgpu.hpp>
#include <webgpu/webgpu.h>


namespace crg::renderer {

    using MaterialID = size_t;

    class MaterialCache {
    public:

        Handle<Material> newMaterial(
            std::string path,
            Buffer cameraUniform,
            RenderContext renderContext,
            MeshBufferSize meshBufferSize,
            std::vector<Buffer>& buffers,
            std::vector<TextureSampler>& samplers,
            std::vector<ImageTexture>& textures
        ) {

            // BIND GROUP LAYOUT ENTRIES:

            size_t chunkCount, instanceCount, mapCount;

            switch (meshBufferSize) {
                case MeshBufferSize::Null:
                   chunkCount = 0;
                   instanceCount = 0;
                   mapCount = 0;
                break;
                case MeshBufferSize::Small:
                   chunkCount = 1024;
                   instanceCount = 32767;
                   mapCount = 256000;
                break;
                case MeshBufferSize::Large:
                    chunkCount = 2048;
                    instanceCount = 65535;
                    mapCount = 512000;
                break;
                default:
                   LOG_CORE_WARNING("Material creation: No mesh buffer size given. Defaulting to Large...");
                   chunkCount = 2048;
                   instanceCount = 65535;
                   mapCount = 512000;
                break;
            }

            MeshBuffer meshBuffer = MeshBuffer(
                renderContext.device,
                renderContext.queue,
                chunkCount,
                instanceCount,
                mapCount
            );

            size_t meshBufferCount = 3;

            size_t bufferCount = buffers.size();

            size_t samplerCount = samplers.size();

            size_t textureCount = textures.size();

            std::vector<wgpu::BindGroupLayoutEntry> layoutEntries(
                meshBufferCount +
                1 +
                bufferCount +
                samplerCount +
                textureCount
            );

            // getCameraBindings(camera);

            getMeshBindings(meshBuffer, layoutEntries);

            getCameraBindings(cameraUniform, layoutEntries);

            getBufferBindings(buffers, layoutEntries, bufferCount, meshBufferCount + 1);

            getSamplerBindings(samplers, layoutEntries, samplerCount, meshBufferCount + 1 + bufferCount);

            getTextureBindings(textures, layoutEntries, textureCount, meshBufferCount + 1 + bufferCount + samplerCount);

            // BIND GROUP LAYOUT:

            const wgpu::BindGroupLayout bindGroupLayout = getBindGroupLayout(renderContext.device, layoutEntries);

            // BIND GROUP ENTRIES:

            std::vector<wgpu::BindGroupEntry> bindGroupEntries = getBindGroupEntries(
                layoutEntries,
                meshBuffer, meshBufferCount,
                cameraUniform,
                buffers, bufferCount,
                samplers, samplerCount,
                textures, textureCount
            );

            // BIND GROUP:
            wgpu::BindGroup bindGroup = getBindGroup(
                bindGroupLayout,
                bindGroupEntries,
                renderContext.device
            );

            // // FILE READING:
            std::ifstream file(path);

            if (!file.is_open()) {
                LOG_CORE_ERROR("Failed to open file");
                return {static_cast<size_t>(-1)};
            }
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            std::string shaderSource(size, ' ');
            file.seekg(0);
            file.read(shaderSource.data(), size);

            // Shader module code:
            wgpu::ShaderSourceWGSL shaderCodeDesc{};
            shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
            shaderCodeDesc.code = wgpu::StringView(shaderSource.c_str());

            wgpu::ShaderModuleDescriptor shaderDesc{};
            shaderDesc.nextInChain = &shaderCodeDesc.chain;

            wgpu::ShaderModule shader = renderContext.device.createShaderModule(shaderDesc);

            // Pipeline code:
            wgpu::PipelineLayoutDescriptor pipelineLayoutDesc{};
            pipelineLayoutDesc.bindGroupLayoutCount = 1;
            pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*) &bindGroupLayout;
            pipelineLayoutDesc.label = wgpu::StringView("Sum pipeline shi");
            pipelineLayoutDesc.nextInChain = nullptr;

            auto pipelineLayout = renderContext.device.createPipelineLayout(pipelineLayoutDesc);



            wgpu::RenderPipelineDescriptor pipelineDesc{};
            pipelineDesc.label = wgpu::StringView("sum pipleine");
            pipelineDesc.layout = pipelineLayout;
            pipelineDesc.depthStencil = &renderContext.depthStencilState;

            // Pipeline states
            wgpu::VertexState vertState{};
            vertState.nextInChain = nullptr;
            vertState.module = shader;
            vertState.entryPoint = wgpu::StringView("vs_main");
            vertState.bufferCount = 0;
            vertState.buffers = nullptr;
            vertState.constantCount = 0;
            vertState.constants = nullptr;

            wgpu::BlendState blendState{};
            blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
            blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
            blendState.color.operation = wgpu::BlendOperation::Add;
            blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
            blendState.alpha.dstFactor = wgpu::BlendFactor::One;
            blendState.alpha.operation = wgpu::BlendOperation::Add;

            std::vector<wgpu::ColorTargetState> colorTargetState{};
            colorTargetState.emplace_back();
            colorTargetState[0].format = renderContext.config.format;
            colorTargetState[0].writeMask = wgpu::ColorWriteMask::All;
            colorTargetState[0].blend = &blendState;

            wgpu::FragmentState fragState{};
            fragState.nextInChain = nullptr;
            fragState.module = shader;
            fragState.entryPoint = wgpu::StringView("fs_main");
            fragState.constantCount = 0;
            fragState.constants = nullptr;
            fragState.targetCount = colorTargetState.size();
            fragState.targets = colorTargetState.data();

            wgpu::PrimitiveState primitiveState{};
            primitiveState.nextInChain = nullptr;
            primitiveState.topology = wgpu::PrimitiveTopology::TriangleList;
            primitiveState.frontFace = wgpu::FrontFace::CCW;
            primitiveState.cullMode = wgpu::CullMode::None;
            primitiveState.unclippedDepth = false;
            primitiveState.stripIndexFormat = wgpu::IndexFormat::Undefined;

            wgpu::MultisampleState multiSampleState{};
            multiSampleState.count = 1;
            multiSampleState.mask = !0;
            multiSampleState.alphaToCoverageEnabled = false;

            pipelineDesc.vertex = vertState;
            pipelineDesc.fragment = &fragState;

            pipelineDesc.primitive = primitiveState;
            pipelineDesc.multisample = multiSampleState;

            wgpu::RenderPipeline pipeline = renderContext.device.createRenderPipeline(pipelineDesc);

            Material material (
                pipeline,
                shader,
                bindGroup,
                bindGroupLayout,
                buffers,
                meshBuffer
            );

            m_materialCache.emplace_back(material);

            return Handle<Material>{ m_materialCache.size() - 1 };
        }

        Material& getMaterial(Handle<Material> handle) {
            return m_materialCache[handle.id];
        }

        std::vector<Material>& getMaterials() {
            return m_materialCache;
        }

    private:
        std::vector<Material> m_materialCache;

        inline void getMeshBindings(
            MeshBuffer& meshBuffer,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries
        ) {
            layoutEntries[0].nextInChain = nullptr;
            layoutEntries[0].binding = 0;
            layoutEntries[0].buffer = meshBuffer.chunkBuffer().getBindingLayout();
            layoutEntries[0].visibility = meshBuffer.chunkBuffer().getStageVisibility();

            layoutEntries[1].nextInChain = nullptr;
            layoutEntries[1].binding = 1;
            layoutEntries[1].buffer = meshBuffer.instanceBuffer().getBindingLayout();
            layoutEntries[1].visibility = meshBuffer.instanceBuffer().getStageVisibility();

            layoutEntries[2].nextInChain = nullptr;
            layoutEntries[2].binding = 2;
            layoutEntries[2].buffer = meshBuffer.meshMapBuffer().getBindingLayout();
            layoutEntries[2].visibility = meshBuffer.meshMapBuffer().getStageVisibility();
        }

        inline void getCameraBindings(
            Buffer& cameraUniform,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries
        ) {
            layoutEntries[3].nextInChain = nullptr;
            layoutEntries[3].binding = 3;
            layoutEntries[3].buffer = cameraUniform.getBindingLayout();
            layoutEntries[3].visibility = cameraUniform.getStageVisibility();
        }


        inline void getBufferBindings(
            std::vector<Buffer>& buffers,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            size_t bufferCount,
            size_t startIdx
        ) {

            // Buffers
            for (size_t i = startIdx; i < startIdx + bufferCount; i++) {
                Buffer& buffer = buffers.at(i - startIdx);

                layoutEntries[i].nextInChain = nullptr;
                layoutEntries[i].binding = i;
                layoutEntries[i].buffer = buffer.getBindingLayout();
                layoutEntries[i].visibility = buffer.getStageVisibility();
            }
        }


        inline void getSamplerBindings(
            std::vector<TextureSampler>& samplers,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            size_t samplerCount,
            size_t startIdx
        ) {
            // Samplers
            for (size_t i = startIdx; i < startIdx + samplerCount; i++) {
                TextureSampler& sampler = samplers.at(i - startIdx);

                layoutEntries[i].nextInChain = nullptr;
                layoutEntries[i].binding = i;
                layoutEntries[i].visibility = sampler.getStageVisibility();
                layoutEntries[i].sampler = sampler.getBindingLayout();
            }
        }

        inline void getTextureBindings(
            std::vector<ImageTexture>& textures,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            size_t textureCount,
            size_t startIdx
        ) {
            // Textures
            for (size_t i = startIdx; i < startIdx + textureCount; i++) {
                ImageTexture& texture = textures.at(i - startIdx);

                layoutEntries[i].nextInChain = nullptr;
                layoutEntries[i].binding = i;
                layoutEntries[i].visibility = texture.getStageVisibility();
                layoutEntries[i].texture = texture.getBindingLayout();
            }

        }

        inline const wgpu::BindGroupLayout getBindGroupLayout(
            wgpu::Device& device,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries
        ) {

            wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
            bindGroupLayoutDesc.nextInChain = nullptr;
            bindGroupLayoutDesc.label = wgpu::StringView("pipa");
            bindGroupLayoutDesc.entryCount = layoutEntries.size();
            bindGroupLayoutDesc.entries = layoutEntries.data();

            return device.createBindGroupLayout(bindGroupLayoutDesc);
        }

        inline std::vector<wgpu::BindGroupEntry> getBindGroupEntries(
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            MeshBuffer& meshBuffer,
            size_t meshBufferCount,
            Buffer& cameraUniform,
            std::vector<Buffer>& buffers,
            size_t bufferCount,
            std::vector<TextureSampler>& samplers,
            size_t samplerCount,
            std::vector<ImageTexture>& textures,
            size_t textureCount
        ) {
            std::vector<wgpu::BindGroupEntry> bindGroupEntries(layoutEntries.size());


            bindGroupEntries[0].nextInChain = nullptr;
            bindGroupEntries[0].binding = 0;
            bindGroupEntries[0].buffer = meshBuffer.chunkBuffer().getRawHandle();
            bindGroupEntries[0].size = meshBuffer.chunkBuffer().getByteSize();
            bindGroupEntries[0].offset = 0;

            bindGroupEntries[1].nextInChain = nullptr;
            bindGroupEntries[1].binding = 1;
            bindGroupEntries[1].buffer = meshBuffer.instanceBuffer().getRawHandle();
            bindGroupEntries[1].size = meshBuffer.instanceBuffer().getByteSize();
            bindGroupEntries[1].offset = 0;

            bindGroupEntries[2].nextInChain = nullptr;
            bindGroupEntries[2].binding = 2;
            bindGroupEntries[2].buffer = meshBuffer.meshMapBuffer().getRawHandle();
            bindGroupEntries[2].size = meshBuffer.meshMapBuffer().getByteSize();
            bindGroupEntries[2].offset = 0;

            bindGroupEntries[3].nextInChain = nullptr;
            bindGroupEntries[3].binding = 3;
            bindGroupEntries[3].buffer = cameraUniform.getRawHandle();
            bindGroupEntries[3].size = cameraUniform.getByteSize();
            bindGroupEntries[3].offset = 0;

            size_t startIdx = meshBufferCount + 1;

            for (size_t i = startIdx; i < startIdx + bufferCount; i++) {
                Buffer& buffer = buffers.at(i - startIdx);

                bindGroupEntries[i].nextInChain = nullptr;
                bindGroupEntries[i].binding = i;
                bindGroupEntries[i].buffer = buffer.getRawHandle();
                bindGroupEntries[i].size = buffer.getByteSize();
                bindGroupEntries[i].offset = 0;
            }

            startIdx += bufferCount;

            for (size_t i = startIdx; i < startIdx + samplerCount; i++) {

                TextureSampler& sampler = samplers.at(i - startIdx);

                bindGroupEntries[i].nextInChain = nullptr;
                bindGroupEntries[i].binding = i;
                bindGroupEntries[i].sampler = sampler.getRawHandle();
            }

            startIdx += samplerCount;

            for (size_t i = startIdx; i < startIdx + textureCount; i++) {

                ImageTexture& texture = textures.at(i - startIdx);

                bindGroupEntries[i].nextInChain = nullptr;
                bindGroupEntries[i].binding = i;
                bindGroupEntries[i].textureView = texture.getTextureView();
                bindGroupEntries[i].offset = 0;
            }

            return bindGroupEntries;
        }


        inline wgpu::BindGroup getBindGroup(
            const wgpu::BindGroupLayout& bindGroupLayout,
            std::vector<wgpu::BindGroupEntry>& bindGroupEntries,
            wgpu::Device& device
        ) {
            wgpu::BindGroupDescriptor bindGroupDesc{};
            bindGroupDesc.nextInChain = nullptr;
            bindGroupDesc.layout = bindGroupLayout;
            bindGroupDesc.entryCount = bindGroupEntries.size();
            bindGroupDesc.entries = bindGroupEntries.data();

            return device.createBindGroup(bindGroupDesc);
        }

    };


}
