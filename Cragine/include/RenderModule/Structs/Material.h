#pragma once
#include <webgpu/webgpu.hpp>
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshBuffer.h"

namespace crg::renderer {

    struct Material {

        Material(
            wgpu::Device& device,
            wgpu::Queue& queue,
            wgpu::RenderPipeline pipeline,
            wgpu::ShaderModule shaderModules,
            wgpu::BindGroup bindGroup,
            wgpu::BindGroupLayout bindGroupLayout,
            std::vector<Buffer>& buffers,
            MeshBuffer& meshBuffer
        ) :
        m_pipeline(pipeline),
        m_shaderModule(shaderModules),
        m_binding(bindGroup),
        m_bindingLayout(bindGroupLayout),
        m_meshBuffer(meshBuffer) {

            for (auto& buffer : buffers) {
                m_buffers.emplace_back(buffer);
            }
        }

        wgpu::RenderPipeline m_pipeline;
        wgpu::ShaderModule m_shaderModule;

        size_t m_totalVertexCount;

        wgpu::BindGroup m_binding;

        wgpu::BindGroupLayout m_bindingLayout;

        MeshBuffer m_meshBuffer;

        std::vector<Buffer> m_buffers;
    };


}
