#pragma once
#include <cstdint>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    struct Material {

        wgpu::RenderPipeline m_pipeline;
        std::vector<wgpu::ShaderModule> m_shaderModules;

        // std::vector<Buffer*> m_buffers;

        uint32_t m_indexCount;

        wgpu::BindGroup m_binding;

        wgpu::BindGroupLayout m_bindingLayout;

        // std::array<wgpu::BindGroup, 4> m_bindings;

        // std::vector<Handle<Buffer>> m_buffers;
        // std::vector<Handle<TextureAtlas>> m_textures;
    };


}
