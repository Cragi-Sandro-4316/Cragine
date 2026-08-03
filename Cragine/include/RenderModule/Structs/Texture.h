#pragma once

#include "utils/Logger.h"
#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image.h>

using namespace glm;

namespace crg::renderer {

    class Texture {
    public:

        Texture(wgpu::Device& device, wgpu::Queue& queue, std::filesystem::path& path) :
        m_shaderStage(wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment) {
            int width;
            int height;
            int channels;
            unsigned char* pixelData = loadTextureData(width, height, channels, path);

            m_textureDesc = wgpu::TextureDescriptor{};
            m_textureDesc.dimension = wgpu::TextureDimension::_2D;
            m_textureDesc.size = { (uint32_t)width, (uint32_t)height, 1 };
            m_textureDesc.mipLevelCount = 1;
            m_textureDesc.sampleCount = 1;
            m_textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
            m_textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
            m_textureDesc.viewFormatCount = 0;
            m_textureDesc.viewFormats = nullptr;

            m_size = m_textureDesc.size;

            if (!pixelData) {
                LOG_CORE_WARNING("Texture path: {} not found, returning default", path.string());
                for (uint32_t i = 0; i < m_textureDesc.size.width; ++i) {
                    for (uint32_t j = 0; j < m_textureDesc.size.height; ++j) {
                        uint8_t *p = &pixelData[4 * (j * m_textureDesc.size.width + i)];
                        p[0] = (i / 16) % 2 == (j / 16) % 2 ? 255 : 0; // r
                        p[1] = ((i - j) / 16) % 2 == 0 ? 255 : 0; // g
                        p[2] = ((i + j) / 16) % 2 == 0 ? 255 : 0; // b
                        p[3] = 255; // a
                    }
                }
            }

            m_texture = device.createTexture(m_textureDesc);

            writeTexture(device, queue, m_textureDesc.mipLevelCount, pixelData);

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
            textureViewDesc.mipLevelCount = m_textureDesc.mipLevelCount;
            textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
            textureViewDesc.format = m_textureDesc.format;

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

        void writeTexture(wgpu::Device& device, wgpu::Queue& queue, uint32_t mipLevelCount, const unsigned char* pixelData) {

            wgpu::TexelCopyTextureInfo destination;
            destination.texture = m_texture;
            destination.mipLevel = 0;
            destination.origin = { 0, 0, 0 };
            destination.aspect = wgpu::TextureAspect::All;

            wgpu::TexelCopyBufferLayout source;
            source.offset = 0;
            source.bytesPerRow = 4 * m_size.width;
            source.rowsPerImage = m_size.height;

            queue.writeTexture(destination, pixelData, 4 * m_size.width * m_size.height, source, m_size);
        }


    private:
        wgpu::Texture m_texture;

        wgpu::TextureDescriptor m_textureDesc;

        wgpu::TextureBindingLayout m_bindingLayout;

        wgpu::ShaderStage m_shaderStage;

        wgpu::TextureView m_textureView;

        wgpu::Extent3D m_size;


        unsigned char* loadTextureData(int& width, int& height, int& channels, std::filesystem::path& path);

    };


}
