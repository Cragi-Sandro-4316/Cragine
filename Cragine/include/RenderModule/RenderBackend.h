#pragma once
#include "RenderModule/Managers/BufferManager.h"
#include "RenderModule/Managers/MaterialCache.h"
#include "RenderModule/Managers/SamplerManager.h"
#include "RenderModule/Managers/TextureManager.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/Texture.h"
#include "Window.h"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {


    class RenderBackend {
    public:

        RenderBackend(Window* window);

        Handle<Material> newMaterial(
            std::string shaderPath,
            std::initializer_list<Handle<Buffer>> buffers,
            std::initializer_list<Handle<TextureSampler>> samplers,
            std::initializer_list<Handle<Texture>> textures
        );


        template<typename T>
        Handle<Buffer> newBuffer(size_t size, BufferType bufferType) {
            wgpu::Device& device = m_renderContext.device;
            wgpu::Queue& queue = m_renderContext.queue;

            return m_bufferManager.newBuffer<T>(size, device, queue, bufferType);
        }

        Handle<TextureSampler> newSampler() {
            wgpu::Device& device = m_renderContext.device;
            wgpu::Queue& queue = m_renderContext.queue;

            return m_samplerManager.newSampler(device, queue);
        }

        Handle<Texture> newTexture(std::filesystem::path path) {
            wgpu::Device& device = m_renderContext.device;
            wgpu::Queue& queue = m_renderContext.queue;

            return m_textureManager.newTexture(device, queue, path);
        }

        template<typename T>
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data) {
            m_bufferManager.writeBuffer(buffer, data);
        }


        RenderContext& getRenderContext() { return m_renderContext; }
        MaterialCache& getMaterialCache() { return m_materialCache; }
        BufferManager& getBufferManager() { return m_bufferManager; }

        Buffer& getBuffer(Handle<Buffer> bufferHandle) {
            return *m_bufferManager.getBufferPtr(bufferHandle);
        }

    private:
        RenderContext m_renderContext;

        MaterialCache m_materialCache{};


        BufferManager m_bufferManager{};

        TextureManager m_textureManager{};

        SamplerManager m_samplerManager{};

    };



}
