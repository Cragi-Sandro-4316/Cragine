#pragma once
#include "RenderModule/Handles.h"
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    struct Material {

        wgpu::RenderPipeline m_pipeline;
        std::vector<wgpu::ShaderModule> m_shaderModules;

        std::vector<Handle<Buffer>> m_buffers;
        std::vector<Handle<TextureAtlas>> m_textures;
    };


}
