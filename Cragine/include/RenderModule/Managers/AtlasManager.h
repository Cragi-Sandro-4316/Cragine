#pragma once
#include "RenderModule/Handles.h"
#include "RenderModule/Structs/TextureAtlas.h"
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    class AtlasManager {
    public:

        Handle<TextureAtlas> newAtlas(size_t pageCount, wgpu::Device& device) {
            m_atlases.emplace_back(TextureAtlas(pageCount, device));

            return Handle<TextureAtlas> {
                .id = m_atlases.size() - 1
            };
        }

        TextureAtlas* getAtlasPtr(Handle<TextureAtlas> handle) {
            return &m_atlases[handle.id];
        }

    private:
        std::vector<TextureAtlas> m_atlases;

    };

}
