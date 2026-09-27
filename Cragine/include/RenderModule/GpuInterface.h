#pragma once
#include "RenderModule/GpuResourceManager.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/GpuResource.h"
#include "RenderModule/Structs/MeshCollection.h"
#include "RenderModule/Structs/TextureAtlas.h"

namespace crg::renderer {


    class GpuInterface {
    public:
        GpuInterface(Window* window) :
        m_renderContext(window) {}

        template<typename T>
        Handle<GpuResource> newBuffer (
            size_t size,
            BufferType bufferType = BufferType::Storage
        ) {
            return m_resourceManager.newBuffer<T>(m_renderContext, size, bufferType);
        }

        Buffer& getBuffer(Handle<GpuResource> handle) {
            return m_resourceManager.getBuffer(handle);
        }


        Handle<GpuResource> newAtlas (
            size_t pageCount
        ) {
            return m_resourceManager.newAtlas(m_renderContext, pageCount);
        }

        TextureAtlas& getAtlas(Handle<GpuResource> handle) {
            return m_resourceManager.getAtlas(handle);
        }

        Handle<AtlasEntry> pushTexture(
            Handle<GpuResource> handle,
            std::filesystem::path path
        ) {
            return m_resourceManager.pushTexture(handle, path, m_renderContext);
        }


        Handle<GpuResource> newMeshCollection (
            MeshCollection::Size size = MeshCollection::Size::Large
        ) {
            return m_resourceManager.newMeshCollection(m_renderContext, size);
        }

        MeshCollection& getMeshCollection(Handle<GpuResource> handle) {
            return m_resourceManager.getMeshCollection(handle);
        }

        Handle<GpuResource> newSampler () {
            return m_resourceManager.newSampler(m_renderContext);
        }

        Sampler& getSampler(Handle<GpuResource> handle) {
            return m_resourceManager.getSampler(handle);
        }

        Handle<GpuResource> newTexture (
            std::filesystem::path path
        ) {
            return m_resourceManager.newTexture(m_renderContext, path);
        }

        ImageTexture& getTexture(Handle<GpuResource> handle) {
            return m_resourceManager.getTexture(handle);
        }

        RenderContext& getRenderContext() {
            return m_renderContext;
        }

        GpuResourceManager& getResourceManager() {
            return m_resourceManager;
        }

    private:
        RenderContext m_renderContext;
        GpuResourceManager m_resourceManager;


    };

}
