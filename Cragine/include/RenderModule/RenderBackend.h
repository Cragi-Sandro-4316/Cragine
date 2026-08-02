#pragma once
#include "RenderModule/Managers/BufferManager.h"
#include "RenderModule/Managers/MaterialCache.h"
#include "RenderModule/Managers/MeshServer.h"
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

        void newMaterial(
            std::string shaderPath,
            size_t indexCount,
            std::initializer_list<Handle<Buffer>> buffers,
            std::initializer_list<Handle<TextureSampler>> samplers,
            std::initializer_list<Handle<Texture>> textures
        );


        Handle<Mesh> loadMesh(std::filesystem::path& path) {
            return m_meshServer.loadMesh(path);
        }

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

        Handle<Texture> newTexture() {
            wgpu::Device& device = m_renderContext.device;
            wgpu::Queue& queue = m_renderContext.queue;

            return m_textureManager.newTexture(device, queue);
        }

        template<typename T>
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data) {
            m_bufferManager.writeBuffer(buffer, data);
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

        TextureManager m_textureManager{};

        SamplerManager m_samplerManager{};

    };



}
