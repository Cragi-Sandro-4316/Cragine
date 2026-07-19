#pragma once
#include "RenderModule/Managers/MaterialCache.h"
#include "RenderModule/RenderContext.h"
#include "Window.h"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {


    class RenderBackend {
    public:

        RenderBackend(Window* window);

        void newMaterial();

        RenderContext& getRenderContext() { return m_renderContext; }
        MaterialCache& getMaterialCache() { return m_materialCache; }

    private:
        RenderContext m_renderContext;

        MaterialCache m_materialCache{};



    };



}
