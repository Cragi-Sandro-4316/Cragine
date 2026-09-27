#pragma once

#include <webgpu/webgpu.hpp>
#include "Ecs/Handle.h"

namespace crg {

    namespace renderer {
        struct Sampler;
    }


    template<>
    struct Handle<renderer::Sampler> {
        size_t id = -1;
    };

    template<typename T> struct is_sampler : std::false_type {};
    template<> struct is_sampler<Handle<renderer::Sampler>> : std::true_type {};


}

namespace crg::renderer {

    class Sampler {
    public:

        Sampler(wgpu::Device& device, wgpu::Queue& queue) {

            wgpu::SamplerDescriptor samplerDesc;
            samplerDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
            samplerDesc.addressModeV = wgpu::AddressMode::ClampToEdge;
            samplerDesc.addressModeW = wgpu::AddressMode::ClampToEdge;
            samplerDesc.magFilter = wgpu::FilterMode::Linear;
            samplerDesc.minFilter = wgpu::FilterMode::Linear;
            samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
            samplerDesc.lodMinClamp = 0.0f;
            samplerDesc.lodMaxClamp = 1.0f;
            samplerDesc.compare = wgpu::CompareFunction::Undefined;
            samplerDesc.maxAnisotropy = 1;

            m_sampler = device.createSampler(samplerDesc);


            m_bindingLayout = wgpu::SamplerBindingLayout{};
            m_bindingLayout.nextInChain = nullptr;
            m_bindingLayout.type = wgpu::SamplerBindingType::Filtering;

            m_shaderStage = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        }

        wgpu::Sampler getRawHandle() {
            return m_sampler;
        }

        wgpu::SamplerBindingLayout getBindingLayout() {
            return m_bindingLayout;
        }

        wgpu::ShaderStage getStageVisibility() {
            return m_shaderStage;
        }

    private:

        wgpu::Sampler m_sampler;

        wgpu::SamplerBindingLayout m_bindingLayout;

        wgpu::ShaderStage m_shaderStage;


    };



}
