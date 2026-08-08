#include "RenderBackend.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/Texture.h"
#include <initializer_list>

namespace crg::renderer {

    RenderBackend::RenderBackend(Window* window) :
    m_renderContext(RenderContext(window)) {}

    void RenderBackend::newMaterial(
        std::string shaderPath,
        size_t indexCount = 0,
        std::initializer_list<Handle<Buffer>> buffers = {},
        std::initializer_list<Handle<TextureSampler>> samplers = {},
        std::initializer_list<Handle<Texture>> textures = {}
    ){

        std::vector<Buffer*> buffs;
        buffs.reserve(buffers.size());
        for (Handle<Buffer> handle : buffers) {
            buffs.push_back(m_bufferManager.getBufferPtr(handle));
        }

        std::vector<TextureSampler*> textureSamplers;
        textureSamplers.reserve(samplers.size());
        for (Handle<TextureSampler> handle : samplers) {
            textureSamplers.push_back(m_samplerManager.getSamplerPtr(handle));
        }

        std::vector<Texture*> texs;
        texs.reserve(textures.size());
        for (Handle<Texture> handle : textures) {
            texs.push_back(m_textureManager.getTexturePtr(handle));
        }

        m_materialCache.newMaterial(
            shaderPath,
            m_renderContext,
            m_meshServer,
            buffs,
            textureSamplers,
            texs,
            indexCount
        );
    }


}
