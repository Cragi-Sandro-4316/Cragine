#pragma once

#include <filesystem>
#include <fstream>
#include <type_traits>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>
#include <boost/pfr/core.hpp>

#include "RenderModule/Structs/MeshCollection.h"
#include "RenderModule/RenderContext.h"
#include "utils/Logger.h"
#include "utils/Assert.h"


namespace crg::renderer {

    struct IMaterial {
        uint32_t getVertexCount() {
            return m_meshCollection->vertexCount();
        }

        MeshCollection* m_meshCollection = nullptr;

        wgpu::BindGroupLayout m_bindingLayout;
        wgpu::BindGroup m_bindGroup;
        wgpu::RenderPipeline m_pipeline;
        wgpu::ShaderModule m_shaderModule;
    };



    template<typename MaterialDef>
    struct Material : IMaterial {

        Material(
            const std::filesystem::path& path,
            RenderContext& renderContext,
            MaterialDef def
        ) :
        m_materialDef(def) {
            std::vector<WGPUBindGroupLayoutEntry> layoutEntries;

            boost::pfr::for_each_field(
                m_materialDef,
                [&](auto& field) {
                    using field_t = std::remove_cvref_t<decltype(field)>;
                    ASSERT(is_gpuResource<field_t>::value, "material {} has non-gpuResource elements", path.c_str());

                    field.bindLayoutEntry(layoutEntries);

                    if constexpr (std::is_same<field_t, MeshCollection>::value) {
                        ASSERT(!m_meshCollection, "MeshCollection for material {} is already set", path.c_str());
                        m_meshCollection = &field;
                    }
                }
            );

            ASSERT(m_meshCollection, "Material {} is missing a mesh collection", path.c_str());

            wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
            bindGroupLayoutDesc.nextInChain = nullptr;
            bindGroupLayoutDesc.label = wgpu::StringView(path.c_str());
            bindGroupLayoutDesc.entryCount = layoutEntries.size();
            bindGroupLayoutDesc.entries = layoutEntries.data();

            m_bindingLayout = renderContext.device.createBindGroupLayout(bindGroupLayoutDesc);


            std::vector<WGPUBindGroupEntry> bindingEntries;

            boost::pfr::for_each_field(
                m_materialDef,
                [&](auto& field) {
                    using field_t = std::remove_cvref_t<decltype(field)>;
                    ASSERT(is_gpuResource<field_t>::value, "material {} has non-gpuResource elements", path.c_str());

                    field.bindEntry(bindingEntries);
                }
            );

            wgpu::BindGroupDescriptor bindGroupDesc{};
            bindGroupDesc.nextInChain = nullptr;
            bindGroupDesc.layout = m_bindingLayout;
            bindGroupDesc.entryCount = bindingEntries.size();
            bindGroupDesc.entries = bindingEntries.data();

            m_bindGroup = renderContext.device.createBindGroup(bindGroupDesc);

            std::ifstream file(path);

            if (!file.is_open()) {
                LOG_CORE_ERROR("Failed to open file {}", path.c_str());
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
            pipelineLayoutDesc.label = wgpu::StringView(path.generic_string().append(" layout"));
            pipelineLayoutDesc.nextInChain = nullptr;

            auto pipelineLayout = renderContext.device.createPipelineLayout(pipelineLayoutDesc);

            wgpu::RenderPipelineDescriptor pipelineDesc{};
            pipelineDesc.label = wgpu::StringView(path.generic_string().append(" pipeline"));
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

        MaterialDef m_materialDef;
    };


}
