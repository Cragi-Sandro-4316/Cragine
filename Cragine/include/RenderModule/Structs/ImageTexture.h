#pragma once
#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image.h>

#include "utils/Logger.h"

using namespace glm;

namespace crg::renderer {

    class ImageTexture {
    public:

        static constexpr WGPUTextureBindingLayout bindingLayout = WGPUTextureBindingLayout {
            .nextInChain = nullptr,
            .sampleType = wgpu::TextureSampleType::Float,
            .viewDimension = wgpu::TextureViewDimension::_2D,
            .multisampled = false
        };

        ImageTexture(wgpu::Device& device, wgpu::Queue& queue, std::filesystem::path& path) :
        m_shaderStage(wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment) {
            int width;
            int height;
            int channels;
            unsigned char* pixelData = loadTextureData(width, height, channels, path);

            LOG_CORE_INFO("texture: {} width and height: [{}, {}]", path.string(), width, height);

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

            m_texture = device.createTexture(m_textureDesc);

            writeTexture(queue, m_textureDesc.mipLevelCount, pixelData);

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

        wgpu::ShaderStage getStageVisibility() {
            return m_shaderStage;
        }

        void writeTexture(wgpu::Queue& queue, uint32_t mipLevelCount, const unsigned char* pixelData) {

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

        void bindLayoutEntry(std::vector<WGPUBindGroupLayoutEntry>& entries) {
            entries.emplace_back(WGPUBindGroupLayoutEntry {
                .nextInChain = nullptr,
                .binding = (uint32_t) entries.size(),
                .visibility = m_shaderStage,
                .texture = bindingLayout
            });
        }

        void bindEntry(std::vector<WGPUBindGroupEntry>& entries) {

            entries.emplace_back(WGPUBindGroupEntry{
                .nextInChain = nullptr,
                .binding = (uint32_t)entries.size(),
                .offset = 0,
                .textureView = m_textureView,
            });

        }

    private:
        wgpu::Texture m_texture;

        wgpu::TextureDescriptor m_textureDesc;

        wgpu::ShaderStage m_shaderStage;

        wgpu::TextureView m_textureView;

        wgpu::Extent3D m_size;


        unsigned char* loadTextureData(int& width, int& height, int& channels, std::filesystem::path& path);

    };



}
