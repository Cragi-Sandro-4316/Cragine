#pragma once

#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Managers/MeshServer.h"
#include "RenderModule/Material/Material.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/Texture.h"

#include "utils/Logger.h"
#include <fstream>
#include <vector>
#include <webgpu/webgpu.hpp>
#include <webgpu/webgpu.h>


namespace crg::renderer {

    using MaterialID = size_t;

    class MaterialCache {
    public:


        MaterialID newMaterial(
            std::string path,
            RenderContext renderContext,
            MeshServer& meshServer,
            std::vector<Buffer*>& buffers,
            std::vector<TextureSampler*>& samplers,
            std::vector<Texture*>& textures,
            size_t indexCount = 0
        ) {
            // TODO: limit to one vertex buffer, one index buffer and one instance buffer.


            LOG_CORE_INFO("Creating material at path: {}", path);

            // BIND GROUP LAYOUT ENTRIES:

            size_t bufferCount = buffers.size();

            size_t samplerCount = samplers.size();

            size_t textureCount = textures.size();

            std::vector<wgpu::BindGroupLayoutEntry> layoutEntries(
                bufferCount +
                samplerCount +
                textureCount
            );

            getBufferBindings(buffers, layoutEntries, bufferCount, 0);

            getSamplerBindings(samplers, layoutEntries, samplerCount, bufferCount);

            getTextureBindings(textures, layoutEntries, textureCount, bufferCount + samplerCount);

            // BIND GROUP LAYOUT:

            const wgpu::BindGroupLayout bindGroupLayout = getBindGroupLayout(renderContext.device, layoutEntries);

            // BIND GROUP ENTRIES:

            std::vector<wgpu::BindGroupEntry> bindGroupEntries = getBindGroupEntries(
                layoutEntries,
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
                return -1;
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

            Material material{};
            material.m_pipeline = pipeline;
            material.m_shaderModules = {shader};
            material.m_binding = bindGroup;
            material.m_bindingLayout = bindGroupLayout;
            material.m_buffers = buffers;

            material.updateCounts();

            m_materialCache.emplace_back(material);

            return m_materialCache.size() - 1;
        }

        const Material& getMaterial(MaterialID matID) const {
            return m_materialCache[matID];
        }

        const std::vector<Material>& getMaterials() {
            return m_materialCache;
        }

    private:
        std::vector<Material> m_materialCache;

        inline void getBufferBindings(
            std::vector<Buffer*>& buffers,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            size_t bufferCount,
            size_t startIdx
        ) {

            // Buffers
            for (size_t i = startIdx; i < startIdx + bufferCount; i++) {
                Buffer& buffer = *buffers.at(i - startIdx);

                layoutEntries[i].nextInChain = nullptr;
                layoutEntries[i].binding = i;
                layoutEntries[i].buffer = buffer.getBindingLayout();
                layoutEntries[i].visibility = buffer.getStageVisibility();
            }
        }


        inline void getSamplerBindings(
            std::vector<TextureSampler*>& samplers,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            size_t samplerCount,
            size_t startIdx
        ) {
            // Samplers
            for (size_t i = startIdx; i < startIdx + samplerCount; i++) {
                TextureSampler& sampler = *samplers.at(i - startIdx);

                layoutEntries[i].nextInChain = nullptr;
                layoutEntries[i].binding = i;
                layoutEntries[i].visibility = sampler.getStageVisibility();
                layoutEntries[i].sampler = sampler.getBindingLayout();
            }
        }

        inline void getTextureBindings(
            std::vector<Texture*>& textures,
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            size_t textureCount,
            size_t startIdx
        ) {
            // Textures
            for (size_t i = startIdx; i < startIdx + textureCount; i++) {
                Texture& texture = *textures.at(i - startIdx);

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
            bindGroupLayoutDesc.entryCount = layoutEntries.size();
            bindGroupLayoutDesc.entries = layoutEntries.data();

            return device.createBindGroupLayout(bindGroupLayoutDesc);
        }

        inline std::vector<wgpu::BindGroupEntry> getBindGroupEntries(
            std::vector<wgpu::BindGroupLayoutEntry>& layoutEntries,
            std::vector<Buffer*>& buffers,
            size_t bufferCount,
            std::vector<TextureSampler*>& samplers,
            size_t samplerCount,
            std::vector<Texture*>& textures,
            size_t textureCount
        ) {
            std::vector<wgpu::BindGroupEntry> bindGroupEntries(layoutEntries.size());

            size_t startIdx = 0;

            for (size_t i = startIdx; i < bufferCount; i++) {
                Buffer& buffer = *buffers.at(i - startIdx);

                bindGroupEntries[i].nextInChain = nullptr;
                bindGroupEntries[i].binding = i;
                bindGroupEntries[i].buffer = buffer.getRawHandle();
                bindGroupEntries[i].size = buffer.getByteSize();
                bindGroupEntries[i].offset = 0;
            }

            startIdx += bufferCount;

            for (size_t i = startIdx; i < startIdx + samplerCount; i++) {
                TextureSampler& sampler = *samplers.at(i - startIdx);

                bindGroupEntries[i].nextInChain = nullptr;
                bindGroupEntries[i].binding = i;
                bindGroupEntries[i].sampler = sampler.getRawHandle();
            }

            startIdx += samplerCount;

            for (size_t i = startIdx; i < startIdx + textureCount; i++) {
                Texture& texture = *textures.at(i - startIdx);

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
