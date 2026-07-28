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

        void newMaterial(std::string shaderPath, std::vector<Handle<Buffer>>& buffers);

        template<typename T>
        Handle<Buffer> newBuffer(size_t size) {
            wgpu::Device& device = m_renderContext.device;
            wgpu::Queue& queue = m_renderContext.queue;

            return m_bufferManager.newBuffer<T>(size, device, queue);
        }

        template<typename T>
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data) {
            m_bufferManager.writeBuffer(buffer, data, m_renderContext.queue);
        }


        RenderContext& getRenderContext() { return m_renderContext; }
        MaterialCache& getMaterialCache() { return m_materialCache; }
        BufferManager& getBufferManager() { return m_bufferManager; }
        MeshServer& getMeshServer() { return m_meshServer; }

        Buffer& getBuffer(Handle<Buffer> bufferHandle) {
            return *m_bufferManager.getBufferPtr(bufferHandle);
        }

    private:
        RenderContext m_renderContext;

        MaterialCache m_materialCache{};

        MeshServer m_meshServer{};

        BufferManager m_bufferManager{};

    };



}
