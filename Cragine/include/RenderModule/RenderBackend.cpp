#include "RenderBackend.h"

namespace crg::renderer {

    RenderBackend::RenderBackend(Window* window) :
    m_renderContext(RenderContext(window)) {}

    void RenderBackend::newMaterial(
        std::string shaderPath,
        std::vector<Handle<Buffer>>* buffers,
        std::vector<Handle<Texture>>* textures
    ){

        std::vector<Buffer*> buffs((buffers) ? buffers->size() : 0);
        for (size_t i = 0; i < buffs.size(); i++) {
            buffs[i] = m_bufferManager.getBufferPtr(buffers->at(i));
        }

        std::vector<Texture*> texs((textures) ? textures->size() : 0);
        for (size_t i = 0; i < texs.size(); i++) {
            texs[i] = m_textureManager.getTexturePtr(textures->at(i));
        }

        m_materialCache.newMaterial(
            shaderPath,
            m_renderContext,
            m_meshServer,
            &buffs,
            nullptr,    // Samplers
            nullptr,    // StorageTextures
            &texs
        );
    }


}
