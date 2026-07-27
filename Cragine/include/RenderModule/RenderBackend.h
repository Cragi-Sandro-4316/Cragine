#pragma once
#include "RenderModule/Managers/BufferManager.h"
#include "RenderModule/Managers/MaterialCache.h"
#include "RenderModule/Managers/MeshServer.h"
#include "RenderModule/RenderContext.h"
#include "Window.h"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {


    class RenderBackend {
    public:

        RenderBackend(Window* window);

        void newMaterial();

        template<typename T>
        Handle<Buffer> newBuffer(size_t size) {
            wgpu::Device& device = m_renderContext.device;

            return m_bufferManager.newBuffer<T>(size, device);
        }

        RenderContext& getRenderContext() { return m_renderContext; }
        MaterialCache& getMaterialCache() { return m_materialCache; }
        BufferManager& getBufferManager() { return m_bufferManager; }

    private:
        RenderContext m_renderContext;

        MaterialCache m_materialCache{};

        MeshServer m_meshServer{};

        BufferManager m_bufferManager{};

    };



}
