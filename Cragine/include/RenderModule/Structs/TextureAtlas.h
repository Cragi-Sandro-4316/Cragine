#pragma once

#include "RenderModule/Handles.h"
#include "RenderModule/Structs/Buffer.h"
#include "utils/Logger.h"
#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>

using namespace glm;

namespace crg::renderer {

    const size_t ATLAS_PAGE_SIZE = 128;

    struct AtlasEntry {
        uint32_t firstPageIdx;
        float32_t pageWidth;
        float32_t pageHeight;
        uint32_t width;
        uint32_t height;

    };


    class TextureAtlas {
    public:


        TextureAtlas(
            size_t pageCount,
            wgpu::Device& device,
            wgpu::Queue& queue
        ) :
        m_pageCapacity(pageCount * pageCount),
        m_pagesPerRow(pageCount),
        m_entries(
            pageCount * pageCount,
            BUFFER_TYPE(AtlasEntry),
            device,
            queue,
            wgpu::BufferUsage::Storage  |
            wgpu::BufferUsage::MapRead  |
            wgpu::BufferUsage::MapWrite |
            wgpu::BufferUsage::CopyDst,
            wgpu::BufferBindingType::Storage,
            BufferType::Storage,
            wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment
        ) {

            m_atlasDesc = wgpu::TextureDescriptor{};
            m_atlasDesc.dimension = wgpu::TextureDimension::_2D;
            m_atlasDesc.size = { (uint32_t) (m_pagesPerRow * ATLAS_PAGE_SIZE), (uint32_t) (m_pagesPerRow * ATLAS_PAGE_SIZE), 1 };
            m_atlasDesc.mipLevelCount = 1;
            m_atlasDesc.sampleCount = 1;
            m_atlasDesc.format = wgpu::TextureFormat::RGBA8Unorm;
            m_atlasDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
            m_atlasDesc.viewFormatCount = 0;
            m_atlasDesc.viewFormats = nullptr;

            m_size = m_atlasDesc.size;

            m_atlas = device.createTexture(m_atlasDesc);

            m_bindingLayout = wgpu::TextureBindingLayout{};
            m_bindingLayout.nextInChain = nullptr;
            m_bindingLayout.multisampled = false;
            m_bindingLayout.sampleType = wgpu::TextureSampleType::Float;
            m_bindingLayout.viewDimension = wgpu::TextureViewDimension::_2D;

            wgpu::TextureViewDescriptor atlasViewDesc{};
            atlasViewDesc.aspect = wgpu::TextureAspect::All;
            atlasViewDesc.baseArrayLayer = 0;
            atlasViewDesc.arrayLayerCount = 1;
            atlasViewDesc.baseMipLevel = 0;
            atlasViewDesc.mipLevelCount = m_atlasDesc.mipLevelCount;
            atlasViewDesc.dimension = wgpu::TextureViewDimension::_2D;
            atlasViewDesc.format = m_atlasDesc.format;

            m_atlasView = m_atlas.createView(atlasViewDesc);

            m_shaderStage = wgpu::ShaderStage::Fragment;
        }


        Handle<AtlasEntry> pushTexture(
            std::filesystem::path& path,
            wgpu::Queue& queue,
            uint32_t mipLevelCount
        ) {
            int textureWidth, textureHeight, channels;
            unsigned char* pixelData = loadTextureData(textureWidth, textureHeight, channels, path);

            uint32_t pageCountX = std::ceil(
                (double)textureWidth /
                (double)ATLAS_PAGE_SIZE
            );

            uint32_t pageCountY = std::ceil(
                (double)textureHeight /
                (double)ATLAS_PAGE_SIZE
            );

            uint32_t texturePageCount = pageCountX * pageCountY;

            if (m_pageCount + texturePageCount > m_pageCapacity) {
                LOG_CORE_ERROR("Atlas cannot fit texture {} of size: ({}, {})", path.c_str(), textureWidth, textureHeight);
                return { .id = (size_t) -1 };
            }

            auto entry = AtlasEntry {
                .firstPageIdx = m_pageCount,
                .pageWidth = (float32_t)textureWidth / (float32_t)ATLAS_PAGE_SIZE,
                .pageHeight = (float32_t)textureHeight / (float32_t)ATLAS_PAGE_SIZE,
                .width = (uint32_t)textureWidth,
                .height = (uint32_t)textureHeight
            };

            LOG_CORE_WARNING("writing buffer index: {}", m_entryCount);
            LOG_CORE_WARNING("first page: {}, width: {}, height: {}", m_pageCount, entry.width, entry.height);
            LOG_CORE_WARNING("Buffer size: {}", m_entries.size());

            m_entries.write(entry, m_entryCount);

            for (size_t pageY = 0; pageY < pageCountY; pageY++) {
                for (size_t pageX = 0; pageX < pageCountX; pageX++) {

                    size_t srcX = pageX * ATLAS_PAGE_SIZE;
                    size_t srcY = pageY * ATLAS_PAGE_SIZE;

                    size_t pageWidth = std::min(ATLAS_PAGE_SIZE, textureWidth - srcX);
                    size_t pageHeight = std::min(ATLAS_PAGE_SIZE, textureHeight - srcY);


                    uint32_t dstX = (m_pageCount % m_pagesPerRow) * ATLAS_PAGE_SIZE;
                    uint32_t dstY = (m_pageCount / m_pagesPerRow) * ATLAS_PAGE_SIZE;


                    const uint8_t* pageData = pixelData + (srcY * textureWidth + srcX) * channels;


                    wgpu::TexelCopyTextureInfo destination;
                    destination.texture = m_atlas;
                    destination.mipLevel = 0;
                    destination.origin = { dstX, dstY, 0 };
                    destination.aspect = wgpu::TextureAspect::All;

                    wgpu::TexelCopyBufferLayout dataLayout;
                    dataLayout.offset = 0;
                    dataLayout.bytesPerRow = textureWidth * channels;
                    dataLayout.rowsPerImage = pageHeight;

                    wgpu::Extent3D writeSize;
                    writeSize.width = pageWidth;
                    writeSize.height = pageHeight;
                    writeSize.depthOrArrayLayers = 1;

                    queue.writeTexture(
                        destination,
                        pageData,
                        textureWidth * textureHeight * channels,
                        dataLayout,
                        writeSize
                    );

                    m_pageCount++;
                }
            }

            return Handle<AtlasEntry> {
                .id = m_entryCount++
            };
        }


        wgpu::Texture getAtlas() {
            return m_atlas;
        }

        wgpu::TextureView getView() {
            return m_atlasView;
        }

        wgpu::TextureBindingLayout getBindingLayout() {
            return m_bindingLayout;
        }

        wgpu::ShaderStage getStageVisibility() {
            return m_shaderStage;
        }

        Buffer& getBuffer() {
            return m_entries;
        }


        void printBuffer() {
            BufferView<AtlasEntry> view = m_entries.getBufferView<AtlasEntry>();

            for (int i = 0; i < m_entryCount; i++) {
                LOG_CORE_INFO("entries[{}]: [first page: {}, page width: {}, page height: {}]",
                    i,
                    view[i].firstPageIdx,
                    view[i].width,
                    view[i].height
                );
            }

        }

    private:

        Buffer m_entries;
        size_t m_entryCount = 0;

        // std::vector<AtlasEntry> m_entries;

        const size_t m_pageCapacity;

        const size_t m_pagesPerRow;

        uint32_t m_pageCount = 0;

        wgpu::Texture m_atlas;

        wgpu::TextureView m_atlasView;

        wgpu::TextureDescriptor m_atlasDesc;

        wgpu::TextureBindingLayout m_bindingLayout;

        wgpu::ShaderStage m_shaderStage;

        wgpu::Extent3D m_size;


        unsigned char* loadTextureData(int& width, int& height, int& channels, std::filesystem::path& path);
    };


    /*  wgpu::TexelCopyTextureInfo destination;
    destination.texture = m_texture;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 };
    destination.aspect = wgpu::TextureAspect::All;

    wgpu::TexelCopyBufferLayout source;
    source.offset = 0;
    source.bytesPerRow = 4 * m_size.width;
    source.rowsPerImage = m_size.height;

    queue.writeTexture(destination, pixelData, 4 * m_size.width * m_size.height, source, m_size); */
}
