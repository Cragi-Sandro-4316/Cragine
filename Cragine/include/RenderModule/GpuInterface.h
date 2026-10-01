// #pragma once

// #include "Window.h"
// #include "Ecs/Ecs.h"
// #include "RenderModule/Handles.h"
// #include "RenderModule/Components/Camera.h"
// #include "RenderModule/GpuResourceManager.h"
// #include "RenderModule/Structs/MeshCollection.h"


// namespace crg::renderer {


//     class GpuInterface {
//     public:
//         GpuInterface(Window* window) :
//         m_renderContext(window) {}

//         GpuResource::Type getHandleType(Handle<GpuResource> handle) {
//             return m_resourceManager.getType(handle);
//         }

//         template<typename T>
//         Handle<GpuResource> newBuffer (
//             size_t size = 1,
//             BufferType bufferType = BufferType::Storage
//         ) {
//             return m_resourceManager.newBuffer<T>(m_renderContext, size, bufferType);
//         }

//         Buffer& getBuffer(Handle<GpuResource> handle) {
//             return m_resourceManager.getBuffer(handle);
//         }

//         Handle<GpuResource> newCamera(
//             Camera& cameraData,
//             Entity& entity
//         ) {
//             return m_resourceManager.newCamera(
//                 m_renderContext,
//                 cameraData,
//                 entity
//             );
//         }

//         CameraBuffer& getCamera(Handle<GpuResource> handle) {
//             return m_resourceManager.getCamera(handle);
//         }

//         Handle<GpuResource> newAtlas (
//             size_t pageCount
//         ) {
//             return m_resourceManager.newAtlas(m_renderContext, pageCount);
//         }

//         TextureAtlas& getAtlas(Handle<GpuResource> handle) {
//             return m_resourceManager.getAtlas(handle);
//         }

//         Handle<AtlasEntry> pushTexture(
//             Handle<GpuResource> handle,
//             std::filesystem::path path
//         ) {
//             return m_resourceManager.pushTexture(handle, path, m_renderContext);
//         }


//         Handle<GpuResource> newMeshCollection (
//             MeshCollection::Size size = MeshCollection::Size::Large
//         ) {
//             return m_resourceManager.newMeshCollection(m_renderContext, size);
//         }

//         MeshCollection& getMeshCollection(Handle<GpuResource> handle) {
//             return m_resourceManager.getMeshCollection(handle);
//         }

//         Handle<GpuResource> newSampler () {
//             return m_resourceManager.newSampler(m_renderContext);
//         }

//         Sampler& getSampler(Handle<GpuResource> handle) {
//             return m_resourceManager.getSampler(handle);
//         }

//         Handle<GpuResource> newTexture (
//             std::filesystem::path path
//         ) {
//             return m_resourceManager.newTexture(m_renderContext, path);
//         }

//         ImageTexture& getTexture(Handle<GpuResource> handle) {
//             return m_resourceManager.getTexture(handle);
//         }

//         RenderContext& getRenderContext() {
//             return m_renderContext;
//         }

//         GpuResourceManager& getResourceManager() {
//             return m_resourceManager;
//         }

//     private:
//         RenderContext m_renderContext;
//         GpuResourceManager m_resourceManager;


//     };

// }
