#pragma once
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/TextureAtlas.h"
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    class AtlasManager {
    public:

        Handle<TextureAtlas> newAtlas(size_t pageCount, wgpu::Device& device, wgpu::Queue& queue) {
            m_atlases.emplace_back(TextureAtlas(pageCount, device, queue));

            return Handle<TextureAtlas> {
                .id = m_atlases.size() - 1
            };
        }

        TextureAtlas& getAtlas(Handle<TextureAtlas> handle) {
            return m_atlases.at(handle.id);
        }

        Handle<AtlasEntry> pushTexture(
            Handle<TextureAtlas> handle,
            std::filesystem::path& path,
            RenderContext& renderContext
        ) {
            auto& atlas = m_atlases.at(handle.id);
            return atlas.pushTexture(
                path,
                renderContext.queue,
                0
            );
        }

    private:
        // TODO: make this an unordered map
        std::vector<TextureAtlas> m_atlases;

    };

}
