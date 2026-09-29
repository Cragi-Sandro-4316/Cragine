#pragma once

#include "RenderModule/Handles.h"
#include "RenderModule/Structs/ImageTexture.h"

namespace crg::renderer {

    class TextureManager {
    public:

        Handle<ImageTexture> newTexture(wgpu::Device& device, wgpu::Queue& queue, std::filesystem::path& path) {

            Handle<ImageTexture> handle {
                .id = m_currentID
            };

            m_textures.insert({m_currentID, ImageTexture(device, queue, path)});

            m_currentID++;

            return handle;
        }


        ImageTexture& getTexture(Handle<ImageTexture> handle) {
            return m_textures.at(handle.id);
        }

        inline bool validateHandle(Handle<ImageTexture> handle) {
            return m_textures.contains(handle.id);
        }

        void deleteTexture(Handle<ImageTexture> handle) {
            if (!validateHandle(handle)) {
                LOG_CORE_WARNING("Texture deletion error: given handle is invalid");
                return;
            }

            m_textures.erase(handle.id);
        }

    private:
        size_t m_currentID = 0;

        std::unordered_map<size_t, ImageTexture> m_textures;
    };


}
