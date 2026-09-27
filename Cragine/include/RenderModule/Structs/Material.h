#pragma once

#include "RenderModule/GpuInterface.h"
#include "RenderModule/GpuResourceManager.h"
#include "RenderModule/Structs/GpuResource.h"
#include <webgpu/webgpu.hpp>
#include <fstream>


namespace crg::renderer {

    struct Material {

        Material(
            const std::filesystem::path& path,
            std::initializer_list<Handle<GpuResource>> res,
            GpuInterface& gpuInterface
        ) :
        m_resources(res){

            GpuResourceManager& resManager = gpuInterface.getResourceManager();
            RenderContext& renderContext = gpuInterface.getRenderContext();

            std::vector<wgpu::BindGroupLayoutEntry> entries;
            entries.reserve(m_resources.size());

            for (auto& handle : m_resources) {
                resManager.getLayout(entries, handle);
            }

            wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
            bindGroupLayoutDesc.nextInChain = nullptr;
            bindGroupLayoutDesc.label = wgpu::StringView("pipa");
            bindGroupLayoutDesc.entryCount = entries.size();
            bindGroupLayoutDesc.entries = entries.data();

            m_bindingLayout = renderContext.device.createBindGroupLayout(bindGroupLayoutDesc);

            std::ifstream file(path);

            if (!file.is_open()) {
                LOG_CORE_ERROR("Failed to open file");
                return;
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

            m_shaderModule = renderContext.device.createShaderModule(shaderDesc);

            // Pipeline code:
            wgpu::PipelineLayoutDescriptor pipelineLayoutDesc{};
            pipelineLayoutDesc.bindGroupLayoutCount = 1;
            pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*) &m_bindingLayout;
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
            vertState.module = m_shaderModule;
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
            fragState.module = m_shaderModule;
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

            m_pipeline = renderContext.device.createRenderPipeline(pipelineDesc);
        }


        size_t m_vertexCount = 0;

        std::vector<Handle<GpuResource>> m_resources;
        wgpu::BindGroupLayout m_bindingLayout;
        wgpu::RenderPipeline m_pipeline;
        wgpu::ShaderModule m_shaderModule;
    };


}
