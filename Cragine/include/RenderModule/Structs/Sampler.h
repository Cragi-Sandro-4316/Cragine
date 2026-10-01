#pragma once

#include <webgpu/webgpu.hpp>
#include "RenderModule/RenderContext.h"

namespace crg::renderer {

    class Sampler {
    public:

        Sampler(RenderContext& renderContext) {

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

            m_sampler = renderContext.device.createSampler(samplerDesc);

            m_bindingLayout = wgpu::SamplerBindingLayout{};
            m_bindingLayout.nextInChain = nullptr;
            m_bindingLayout.type = wgpu::SamplerBindingType::Filtering;
        }

        wgpu::Sampler getRawHandle() {
            return m_sampler;
        }

        wgpu::SamplerBindingLayout getBindingLayout() {
            return m_bindingLayout;
        }

        void bindEntry(std::vector<WGPUBindGroupEntry>& entries) {

            entries.emplace_back(WGPUBindGroupEntry{
                .nextInChain = nullptr,
                .binding = (uint32_t)entries.size(),
                .sampler = m_sampler,
            });

        }

    private:

        wgpu::Sampler m_sampler;

        wgpu::SamplerBindingLayout m_bindingLayout;
    };



}
