#pragma once

#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>

using namespace glm;

namespace crg::renderer {

    class Texture {
    public:

        Texture(wgpu::Device& device, wgpu::Queue& queue) :
        m_shaderStage(wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment) {
            wgpu::TextureDescriptor textureDesc{};
            textureDesc.dimension = wgpu::TextureDimension::_2D;
            textureDesc.size = { 256, 256, 1 };
            textureDesc.mipLevelCount = 1;
            textureDesc.sampleCount = 1;
            textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
            textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
            textureDesc.viewFormatCount = 0;
            textureDesc.viewFormats = nullptr;

            // Create image data
            std::vector<uint8_t> pixels(4 * textureDesc.size.width * textureDesc.size.height);
            for (uint32_t i = 0; i < textureDesc.size.width; ++i) {
                for (uint32_t j = 0; j < textureDesc.size.height; ++j) {
                    uint8_t *p = &pixels[4 * (j * textureDesc.size.width + i)];
                    p[0] = (uint8_t)i; // r
                    p[1] = (uint8_t)j; // g
                    p[2] = 128; // b
                    p[3] = 255; // a
                }
            }

            m_texture = device.createTexture(textureDesc);

            wgpu::TexelCopyTextureInfo destination{};
            destination.texture = m_texture;
            destination.mipLevel = 0;
            destination.origin = { 0, 0, 0 };
            destination.aspect = wgpu::TextureAspect::All;

            wgpu::TexelCopyBufferLayout source{};
            source.offset = 0;
            source.bytesPerRow = 4 * textureDesc.size.width;
            source.rowsPerImage = textureDesc.size.height;

            queue.writeTexture(
                destination,
                pixels.data(),
                pixels.size(),
                source,
                textureDesc.size
            );

            m_bindingLayout = wgpu::TextureBindingLayout{};
            m_bindingLayout.nextInChain = nullptr;
            m_bindingLayout.multisampled = false;
            m_bindingLayout.sampleType = wgpu::TextureSampleType::Float;
            m_bindingLayout.viewDimension = wgpu::TextureViewDimension::_2D;

            wgpu::TextureViewDescriptor textureViewDesc{};
            textureViewDesc.aspect = wgpu::TextureAspect::All;
            textureViewDesc.baseArrayLayer = 0;
            textureViewDesc.arrayLayerCount = 1;
            textureViewDesc.baseMipLevel = 0;
            textureViewDesc.mipLevelCount = 1;
            textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
            textureViewDesc.format = textureDesc.format;

            m_textureView = m_texture.createView(textureViewDesc);
        }

        wgpu::Texture getRawHandle() {
            return m_texture;
        }

        wgpu::TextureView getTextureView() {
            return m_textureView;
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

        wgpu::TextureView m_textureView;

        vec2 m_size;

    };


}
