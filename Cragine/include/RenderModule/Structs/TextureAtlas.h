#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>

#include "Ecs/Handle.h"

#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/BufferView.h"

using namespace glm;


namespace crg::renderer {

    const size_t ATLAS_PAGE_SIZE = 128;
    const size_t ATLAS_PAGE_LENGTH = 8;

    struct AtlasEntry {
        uint32_t firstPageIdx;
        float32_t pageWidth;
        float32_t pageHeight;
        uint32_t width;
        uint32_t height;
    };

    struct AtlasMetadata {
        uint32_t pageLenght;
        AtlasEntry entries[ATLAS_PAGE_LENGTH * ATLAS_PAGE_LENGTH];
    };

    class TextureAtlas {
    public:

        static constexpr WGPUTextureBindingLayout bindingLayout = {
            .nextInChain = nullptr,
            .sampleType = wgpu::TextureSampleType::Float,
            .viewDimension = wgpu::TextureViewDimension::_2D,
            .multisampled = false
        };

        static constexpr wgpu::BufferBindingType bufferBindingType = wgpu::BufferBindingType::Storage;
        using BufferType = AtlasMetadata;


        TextureAtlas(
            RenderContext& renderContext
        ) :
        m_metadata(
            renderContext,
            wgpu::BufferUsage::Storage  |
            wgpu::BufferUsage::MapRead  |
            wgpu::BufferUsage::MapWrite |
            wgpu::BufferUsage::CopyDst
        ) {

            wgpu::TextureDescriptor atlasDesc{};
            atlasDesc.dimension = wgpu::TextureDimension::_2D;
            atlasDesc.size = { (uint32_t) (m_pageLength * ATLAS_PAGE_SIZE), (uint32_t) (m_pageLength * ATLAS_PAGE_SIZE), 1 };
            atlasDesc.mipLevelCount = 1;
            atlasDesc.sampleCount = 1;
            atlasDesc.format = wgpu::TextureFormat::RGBA8Unorm;
            atlasDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
            atlasDesc.viewFormatCount = 0;
            atlasDesc.viewFormats = nullptr;

            m_size = atlasDesc.size;

            m_atlas = renderContext.device.createTexture(atlasDesc);

            wgpu::TextureViewDescriptor atlasViewDesc{};
            atlasViewDesc.aspect = wgpu::TextureAspect::All;
            atlasViewDesc.baseArrayLayer = 0;
            atlasViewDesc.arrayLayerCount = 1;
            atlasViewDesc.baseMipLevel = 0;
            atlasViewDesc.mipLevelCount = atlasDesc.mipLevelCount;
            atlasViewDesc.dimension = wgpu::TextureViewDimension::_2D;
            atlasViewDesc.format = atlasDesc.format;

            m_atlasView = m_atlas.createView(atlasViewDesc);

            BufferView<AtlasMetadata> view = m_metadata.getBufferView();
            view.get()->pageLenght = m_pageLength;
        }


        Handle<AtlasEntry> pushTexture(
            std::filesystem::path path,
            wgpu::Queue& queue,
            uint32_t mipLevelCount
        ) {
            BufferView<AtlasMetadata> view = m_metadata.getBufferView();
            AtlasMetadata* metadata = view.get();

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

            metadata->entries[m_entryCount] = entry;

            for (size_t pageY = 0; pageY < pageCountY; pageY++) {
                for (size_t pageX = 0; pageX < pageCountX; pageX++) {

                    size_t srcX = pageX * ATLAS_PAGE_SIZE;
                    size_t srcY = pageY * ATLAS_PAGE_SIZE;

                    size_t pageWidth = std::min(ATLAS_PAGE_SIZE, textureWidth - srcX);
                    size_t pageHeight = std::min(ATLAS_PAGE_SIZE, textureHeight - srcY);

                    uint32_t dstX = (m_pageCount % m_pageLength) * ATLAS_PAGE_SIZE;
                    uint32_t dstY = (m_pageCount / m_pageLength) * ATLAS_PAGE_SIZE;

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

        wgpu::TextureView getView() {
            return m_atlasView;
        }

        auto& getBuffer() {
            return m_metadata;
        }
    private:

        Buffer<wgpu::BufferBindingType::Storage, AtlasMetadata> m_metadata;
        size_t m_entryCount = 0;
        uint32_t m_pageCount = 0;

        const size_t m_pageCapacity = ATLAS_PAGE_LENGTH * ATLAS_PAGE_LENGTH;

        const size_t m_pageLength = ATLAS_PAGE_LENGTH;

        wgpu::Texture m_atlas;

        wgpu::TextureView m_atlasView;

        wgpu::Extent3D m_size;

        unsigned char* loadTextureData(int& width, int& height, int& channels, std::filesystem::path& path);
    };

}
