#include "RenderBackend.h"

namespace crg::renderer {

    RenderBackend::RenderBackend(Window* window) :
    m_renderContext(RenderContext(window)) {}

    void RenderBackend::newMaterial(){
        m_materialCache.newMaterial(m_renderContext, m_meshServer);
    }


}
