#pragma once

#include "RenderModule/Structs/Texture.h"
#include "utils/Logger.h"
#include "RenderModule/Handles.h"

namespace crg::renderer {


    class TextureManager {
    public:

        Handle<Texture> newTexture(wgpu::Device& device, wgpu::Queue& queue, std::filesystem::path& path) {

            Handle<Texture> handle {
                .id = m_currentID
            };

            m_textures.insert({m_currentID, Texture(device, queue, path)});

            m_currentID++;

            return handle;
        }


        Texture* getTexturePtr(Handle<Texture> handle) {
            auto it = m_textures.find(handle.id);

            if (it == m_textures.end()) {
                LOG_CORE_ERROR("Gpu getBuffer error: given handle is invalid");
                return nullptr;
            }

            return &it->second;
        }

        inline bool validateHandle(Handle<Texture> handle) {
            return m_textures.contains(handle.id);
        }

        void deleteTexture(Handle<Texture> handle) {
            if (!validateHandle(handle)) {
                LOG_CORE_WARNING("Texture deletion error: given handle is invalid");
                return;
            }

            m_textures.erase(handle.id);
        }

    private:
        size_t m_currentID = 0;

        std::unordered_map<size_t, Texture> m_textures;
    };



}
