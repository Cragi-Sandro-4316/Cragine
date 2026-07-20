#include "RenderBackend.h"
#include "RenderModule/Managers/MaterialCache.h"
#include "RenderModule/RenderContext.h"

namespace crg::renderer {

    RenderBackend::RenderBackend(Window* window) :
    m_renderContext(RenderContext(window)) {}


    void RenderBackend::newMaterial() {
        m_materialCache.newMaterial(m_renderContext, m_meshServer);
    }



}
