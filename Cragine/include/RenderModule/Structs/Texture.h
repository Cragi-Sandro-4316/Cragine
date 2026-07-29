#pragma once

#include <webgpu/webgpu.hpp>
namespace crg::renderer {

    class Texture {
    public:
        wgpu::Texture getRawHandle() {
            return m_texture;
        }

        wgpu::TextureBindingLayout getBindingLayout() {
            return m_bindingLayout;
        }

        wgpu::ShaderStage getStageVisibility() {
            return m_shaderStage;
        }

    private:
        wgpu::Texture m_texture;

        wgpu::TextureBindingLayout m_bindingLayout;

        wgpu::ShaderStage m_shaderStage;
    };


}
