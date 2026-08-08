#pragma once
#include <cstdint>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    struct Material {

        wgpu::RenderPipeline m_pipeline;
        std::vector<wgpu::ShaderModule> m_shaderModules;


        uint32_t m_indexCount;

        wgpu::BindGroup m_binding;

        wgpu::BindGroupLayout m_bindingLayout;

    };


}
