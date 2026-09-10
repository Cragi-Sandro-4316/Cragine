#pragma once
#include "RenderModule/Components/Camera.h"
#include "RenderModule/Managers/AtlasManager.h"
#include "RenderModule/Managers/BufferManager.h"
#include "RenderModule/Managers/MaterialCache.h"
#include "RenderModule/Managers/SamplerManager.h"
#include "RenderModule/Managers/TextureManager.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/MeshBuffer.h"
#include "RenderModule/Structs/ImageTexture.h"
#include "RenderModule/Structs/TextureAtlas.h"
#include "Window.h"
#include "utils/Logger.h"
#include "Components/Transform.h"

#include <filesystem>
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
            Camera camera,
            MeshBufferSize meshBufferSize,
            GpuResources... resources
        ){
            std::vector<Buffer> buffers;
            std::vector<ImageTexture> textures;
            std::vector<TextureSampler> samplers;
            std::vector<TextureAtlas> atlases;

            (appendResource(buffers, samplers, textures, atlases, resources), ...);

            auto cameraHandle = m_bufferManager.newBuffer<CameraUniform>(
                1,
                m_renderContext.device,
                m_renderContext.queue,
                BufferType::Uniform
            );

            auto cameraUniform = m_bufferManager.getBufferPtr(cameraHandle);

            auto cameraData = CameraUniform {
                camera.getProjection()
            };

            cameraUniform->write(cameraData);

            return m_materialCache.newMaterial(
                shaderPath,
                *cameraUniform,
                m_renderContext,
                meshBufferSize,
                buffers,
                samplers,
                textures,
                atlases
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

        Handle<ImageTexture> newTexture(std::filesystem::path path) {
            wgpu::Device& device = m_renderContext.device;
            wgpu::Queue& queue = m_renderContext.queue;

            return m_textureManager.newTexture(device, queue, path);
        }


        Handle<TextureAtlas> newAtlas(size_t pageCount) {
            return m_atlasManager.newAtlas(pageCount, m_renderContext.device, m_renderContext.queue);
        }

        void writeAtlas(Handle<TextureAtlas> handle, std::filesystem::path path) {
            m_atlasManager.getAtlasPtr(handle)->pushTexture(path, m_renderContext.queue, 0);
        }


        template<typename T>
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data) {
            m_bufferManager.writeBuffer(buffer, data);
        }

        Handle<Mesh> spawnMesh(const std::filesystem::path& path, Handle<Material> materialHandle, Transform transform) {
            auto& material = m_materialCache.getMaterial(materialHandle);

            Handle<Mesh> meshHandle = material.m_meshBuffer.loadMesh(path, transform);

            m_meshMap[meshHandle.id] = materialHandle;

            return meshHandle;
        }

        void unloadMesh(Handle<Mesh> mesh) {
            auto it = m_meshMap.find(mesh.id);
            if (it == m_meshMap.end()) {
                LOG_CORE_WARNING("Mesh unloading: mesh handle not found. Skipping...");
                return;
            }

            auto& material = m_materialCache.getMaterial(it->second);
            // material.m_meshBuffer.unloadMesh(mesh);
        }

        void deleteInstance(Handle<Mesh> mesh) {
            auto it = m_meshMap.find(mesh.id);
            if (it == m_meshMap.end()) {
                LOG_CORE_WARNING("Mesh unloading: mesh handle not found. Skipping...");
                return;
            }

            auto& material = m_materialCache.getMaterial(it->second);
            material.m_meshBuffer.deleteInstance(mesh);
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

        AtlasManager m_atlasManager{};

        MaterialCache m_materialCache{};

        TextureManager m_textureManager{};

        SamplerManager m_samplerManager{};


        std::unordered_map<size_t, Handle<Material>> m_meshMap;

        template<typename T>
        inline void appendResource(
            std::vector<Buffer>& buffers,
            std::vector<TextureSampler>& samplers,
            std::vector<ImageTexture>& textures,
            std::vector<TextureAtlas>& atlases,
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
            else if constexpr (is_atlas<T>::value) {
                atlases.push_back(
                    *m_atlasManager.getAtlasPtr(resource)
                );
            }
        }

    };



}
