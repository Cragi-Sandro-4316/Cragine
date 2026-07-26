#pragma once

#include "RenderModule/Managers/MeshServer.h"
#include "RenderModule/Material/Material.h"
#include "RenderModule/RenderContext.h"
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
            RenderContext renderContext,
            MeshServer& meshServer
        ) {

            const char* path = "../assets/fragVert.wgsl";


            // GET VERTICES:
            auto [buffer, buffDesc] = meshServer.newTri(
                renderContext.device,
                renderContext.queue
            );

            // Vertex attributes:

            std::vector<wgpu::VertexAttribute> vertAttributes;
            // Position
            vertAttributes.emplace_back(WGPUVertexAttribute {
                .format = wgpu::VertexFormat::Float32x3,
                .offset = offsetof(Vertex, position),
                .shaderLocation = 0
            });

            // Color
            vertAttributes.emplace_back(WGPUVertexAttribute {
                .format = wgpu::VertexFormat::Float32x3,
                .offset = offsetof(Vertex, color),
                .shaderLocation = 1
            });

            wgpu::VertexBufferLayout buffLayout{};
            buffLayout.arrayStride = sizeof(Vertex);
            buffLayout.stepMode = wgpu::VertexStepMode::Vertex;
            buffLayout.attributeCount = vertAttributes.size();
            buffLayout.attributes = vertAttributes.data();



            // FILE READING:
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
            pipelineLayoutDesc.bindGroupLayoutCount = 0;
            pipelineLayoutDesc.bindGroupLayouts = nullptr;
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
            primitiveState.cullMode = wgpu::CullMode::Back;
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
            material.m_buffer = buffer;
            material.m_buffDesc = buffDesc;
            material.m_vertexCount = 3;

            m_materialCache.emplace_back(material);

            return m_materialCache.size() - 1;
        }

        const Material& getMaterial(MaterialID matID) const {
            return m_materialCache[matID];
        }


    private:
        std::vector<Material> m_materialCache;
    };



}
