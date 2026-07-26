#pragma once
#include <cstdint>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    struct Material {

        wgpu::RenderPipeline m_pipeline;
        std::vector<wgpu::ShaderModule> m_shaderModules;


        wgpu::Buffer m_buffer;
        wgpu::BufferDescriptor m_buffDesc;

        uint32_t m_vertexCount;

        wgpu::BindGroup m_binding;

        // std::array<wgpu::BindGroup, 4> m_bindings;

        // std::vector<Handle<Buffer>> m_buffers;
        // std::vector<Handle<TextureAtlas>> m_textures;
    };


}
