#pragma once

#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    class DepthTexture {
    public:

        DepthTexture() {}

        void initialize(
            wgpu::Device& device,
            wgpu::SurfaceConfiguration& config
        ) {

            wgpu::TextureDescriptor depthTextureDesc{};
            depthTextureDesc.nextInChain = nullptr;
            depthTextureDesc.label = wgpu::StringView("Depth texture");
            depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
            depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
            depthTextureDesc.size = { config.width, config.height, 1 };
            depthTextureDesc.format = DEPTH_FORMAT;
            depthTextureDesc.mipLevelCount = 1;
            depthTextureDesc.sampleCount = 1;

            m_depthTexture = device.createTexture(depthTextureDesc);

            wgpu::TextureViewDescriptor depthViewDesc{};
            depthViewDesc.nextInChain = nullptr;
            depthViewDesc.label = wgpu::StringView("Depth Texture View");
            depthViewDesc.format = WGPUTextureFormat_Depth24Plus;
            depthViewDesc.dimension = WGPUTextureViewDimension_2D;
            depthViewDesc.baseMipLevel = 0;
            depthViewDesc.mipLevelCount = 1;
            depthViewDesc.baseArrayLayer = 0;
            depthViewDesc.arrayLayerCount = 1;
            depthViewDesc.aspect = WGPUTextureAspect_DepthOnly;

            m_depthView = m_depthTexture.createView(depthViewDesc);
        }


        const wgpu::TextureFormat getFormat() {
            return DEPTH_FORMAT;
        }

        wgpu::Texture getTexture() {
            return m_depthTexture;
        }

        wgpu::TextureView getView() {
            return m_depthView;
        }

    private:

        const wgpu::TextureFormat DEPTH_FORMAT = wgpu::TextureFormat::Depth24Plus;

        wgpu::TextureDescriptor m_textureDesc;

        wgpu::Texture m_depthTexture;

        wgpu::TextureView m_depthView;
    };

}
