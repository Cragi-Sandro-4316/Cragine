#pragma once

#include <webgpu/webgpu.hpp>
namespace crg::renderer {

    class TextureSampler {
    public:
        wgpu::Sampler getRawHandle() {
            return m_sampler;
        }

        wgpu::SamplerBindingLayout getBindingLayout() {
            return m_bindingLayout;
        }

        wgpu::ShaderStage getStageVisibility() {
            return m_shaderStage;
        }

        // size_t getByteSize() {
        //     return m_size * m_typeDesc.size;
        // }
    private:

        wgpu::Sampler m_sampler;

        wgpu::SamplerBindingLayout m_bindingLayout;

        wgpu::ShaderStage m_shaderStage;


    };


}
