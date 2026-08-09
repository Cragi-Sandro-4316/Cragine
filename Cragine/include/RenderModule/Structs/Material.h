#pragma once
#include <webgpu/webgpu.hpp>
#include "RenderModule/Structs/Buffer.h"

namespace crg::renderer {

    struct Material {

        wgpu::RenderPipeline m_pipeline;
        std::vector<wgpu::ShaderModule> m_shaderModules;

        size_t m_totalVertexCount;

        wgpu::BindGroup m_binding;

        wgpu::BindGroupLayout m_bindingLayout;

        std::vector<Buffer*> m_buffers;
    };


}
