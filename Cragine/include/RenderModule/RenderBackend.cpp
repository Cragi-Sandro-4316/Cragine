#include "RenderBackend.h"

namespace crg::renderer {

    RenderBackend::RenderBackend(Window* window) :
    m_renderContext(RenderContext(window)) {}

    void RenderBackend::newMaterial(std::string shaderPath, std::vector<Handle<Buffer>>& buffers){

        std::vector<Buffer*> buffs(buffers.size());
        for (size_t i = 0; i < buffs.size(); i++) {
            buffs[i] = m_bufferManager.getBufferPtr(buffers[i]);
        }

        m_materialCache.newMaterial(shaderPath, m_renderContext, m_meshServer, &buffs);
    }


}
