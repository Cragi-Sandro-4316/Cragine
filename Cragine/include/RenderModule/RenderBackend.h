#pragma once
#include "RenderModule/Managers/BufferManager.h"
#include "RenderModule/Managers/MaterialCache.h"
#include "RenderModule/Managers/SamplerManager.h"
#include "RenderModule/Managers/TextureManager.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/Structs/Texture.h"
#include "Window.h"
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {


    class RenderBackend {
    public:

        RenderBackend(Window* window) :
        m_renderContext(RenderContext(window)) {}

        template<typename... GpuResources>
        Handle<Material> newMaterial(
            std::string shaderPath,
            MeshBufferSize meshBufferSize,
            GpuResources... resources
        ){
            std::vector<Buffer> buffers;
            std::vector<Texture> textures;
            std::vector<TextureSampler> samplers;

            (appendResource(buffers, samplers, textures, resources), ...);

            return m_materialCache.newMaterial(
                shaderPath,
                m_renderContext,
                meshBufferSize,
                buffers,
                samplers,
                textures
            );
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

        Handle<Texture> newTexture(std::filesystem::path path) {
            wgpu::Device& device = m_renderContext.device;
            wgpu::Queue& queue = m_renderContext.queue;

            return m_textureManager.newTexture(device, queue, path);
        }

        template<typename T>
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data) {
            m_bufferManager.writeBuffer(buffer, data);
        }

        Handle<Mesh> spawnMesh(const std::filesystem::path& path, Handle<Material> handle) {
            auto& material = m_materialCache.getMaterial(handle);

            return material.m_meshBuffer.loadMesh(path);
        }


        RenderContext& getRenderContext() { return m_renderContext; }
        MaterialCache& getMaterialCache() { return m_materialCache; }
        BufferManager& getBufferManager() { return m_bufferManager; }

        Buffer& getBuffer(Handle<Buffer> bufferHandle) {
            return *m_bufferManager.getBufferPtr(bufferHandle);
        }

    private:
        RenderContext m_renderContext;

        BufferManager m_bufferManager{};


        MaterialCache m_materialCache{};

        TextureManager m_textureManager{};

        SamplerManager m_samplerManager{};




        template<typename T>
        inline void appendResource(
            std::vector<Buffer>& buffers,
            std::vector<TextureSampler>& samplers,
            std::vector<Texture>& textures,
            const T& resource
        ) {
            if constexpr (is_buffer<T>::value) {
                buffers.push_back(
                    *m_bufferManager.getBufferPtr(resource)
                );
            }
            else if constexpr (is_sampler<T>::value) {
                samplers.push_back(
                    *m_samplerManager.getSamplerPtr(resource)
                );
            }
            else if constexpr (is_texture<T>::value) {
                textures.push_back(
                    *m_textureManager.getTexturePtr(resource)
                );
            }
        }

    };



}
