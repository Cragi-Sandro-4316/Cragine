#pragma once

#include "RenderModule/Handles.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <webgpu/webgpu.hpp>

using namespace glm;

namespace crg::renderer {

    const size_t ATLAS_PAGE_SIZE = 128;

    struct AtlasEntry {
        size_t firstPageIdx;
        size_t pageCount;
    };


    class TextureAtlas {
    public:


        TextureAtlas(
            size_t pageCount,
            wgpu::Device& device
        ) :
        m_pageCapacity(ATLAS_PAGE_SIZE * pageCount),
        m_pagesPerRow(pageCount) {

            m_atlasDesc = wgpu::TextureDescriptor{};
            m_atlasDesc.dimension = wgpu::TextureDimension::_2D;
            m_atlasDesc.size = { (uint32_t) m_pageCapacity, (uint32_t) m_pageCapacity, 1 };
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

            size_t pageCountX = std::ceil(
                (double)textureWidth /
                (double)ATLAS_PAGE_SIZE
            );

            size_t pageCountY = std::ceil(
                (double)textureHeight /
                (double)ATLAS_PAGE_SIZE
            );

            size_t texturePageCount = pageCountX + pageCountY;




            for (size_t py = 0; py < ATLAS_PAGE_SIZE; py++) {
                for (size_t px = 0; px < ATLAS_PAGE_SIZE; px++) {


                }
            }



            return Handle<AtlasEntry> {
                .id = m_entries.size() - 1
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

    private:


        std::vector<AtlasEntry> m_entries;

        const size_t m_pageCapacity;

        const size_t m_pagesPerRow;

        size_t m_pageCount = 0;

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
