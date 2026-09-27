#pragma once

#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/GpuResource.h"
#include "RenderModule/Managers/AtlasManager.h"
#include "RenderModule/Managers/BufferManager.h"
#include "RenderModule/Managers/MeshManager.h"
#include "RenderModule/Managers/SamplerManager.h"
#include "RenderModule/Managers/TextureManager.h"
#include "RenderModule/Structs/ImageTexture.h"
#include "RenderModule/Structs/MeshCollection.h"
#include "RenderModule/Structs/TextureAtlas.h"
#include "utils/Assert.h"
#include <unordered_map>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    using GpuResourceID = size_t;

    class GpuResourceManager {
    public:

        GpuResource::Type getType(Handle<GpuResource> handle) {
            return m_resources.at(handle.id).type;
        }

        GpuResource& getResource(Handle<GpuResource> handle) {
            return m_resources.at(handle.id);
        }

        template<typename T>
        Handle<GpuResource> newBuffer (
            RenderContext& renderContext,
            size_t size,
            BufferType bufferType = BufferType::Storage
        ) {
            Handle<GpuResource> handle{
                .id = m_nextID++
            };

            m_resources.emplace(
                handle.id,
                GpuResource {
                    .type = GpuResource::Type::Buffer,
                    .buffer = m_bufferManager.newBuffer<T>(
                        size,
                        renderContext.device,
                        renderContext.queue,
                        bufferType
                    )
                }
            );

            return handle;
        }

        Buffer& getBuffer(Handle<GpuResource> handle) {
            GpuResource buffer = m_resources.at(handle.id);

            ASSERT(buffer.type == GpuResource::Type::Buffer, "Buffer fetch: Given handle was not buffer handle.", 0);

            return m_bufferManager.getBuffer(buffer.buffer);
        }

        Handle<GpuResource> newAtlas (
            RenderContext& renderContext,
            size_t pageCount
        ) {
            Handle<GpuResource> handle{
                .id = m_nextID++
            };

            m_resources.emplace(
                handle.id,
                GpuResource {
                    .type = GpuResource::Type::Atlas,
                    .atlas = m_atlasManager.newAtlas(
                        pageCount,
                        renderContext.device,
                        renderContext.queue
                    )
                }
            );

            return handle;
        }

        TextureAtlas& getAtlas(Handle<GpuResource> handle) {
            GpuResource atlas = m_resources.at(handle.id);

            ASSERT(atlas.type == GpuResource::Type::Atlas, "Atlas fetch: Given handle was not atlas handle.", 0);

            return m_atlasManager.getAtlas(atlas.atlas);
        }

        Handle<AtlasEntry> pushTexture(Handle<GpuResource> handle, std::filesystem::path& path, RenderContext& renderContext) {
            auto& resource = m_resources.at(handle.id);

            ASSERT(resource.type == GpuResource::Type::Atlas, "Atlas write: Given handle was not an atlas handle", 0);

            return m_atlasManager.pushTexture(resource.atlas, path, renderContext);
        }


        Handle<GpuResource> newMeshCollection (
            RenderContext& renderContext,
            MeshCollection::Size size = MeshCollection::Size::Large
        ) {
            Handle<GpuResource> handle{
                .id = m_nextID++
            };

            m_resources.emplace(
                handle.id,
                GpuResource {
                    .type = GpuResource::Type::MeshCollection,
                    .meshCollection = m_meshManager.newCollection(
                        renderContext.device,
                        renderContext.queue,
                        size
                    )
                }
            );

            return handle;
        }

        MeshCollection& getMeshCollection(Handle<GpuResource> handle) {
            GpuResource resource = m_resources.at(handle.id);

            ASSERT(resource.type == GpuResource::Type::MeshCollection, "Given handle was not buffer handle.", 0);

            return m_meshManager.getCollection(resource.meshCollection);
        }

        Handle<GpuResource> newSampler (
            RenderContext& renderContext
        ) {
            Handle<GpuResource> handle{
                .id = m_nextID++
            };

            m_resources.emplace(
                handle.id,
                GpuResource {
                    .type = GpuResource::Type::Sampler,
                    .sampler = m_samplerManager.newSampler(
                        renderContext.device,
                        renderContext.queue
                    )
                }
            );

            return handle;
        }

        Sampler& getSampler(Handle<GpuResource> handle) {
            GpuResource resource = m_resources.at(handle.id);

            ASSERT(resource.type == GpuResource::Type::Sampler, "Given handle was not buffer handle.", 0);

            return m_samplerManager.getSampler(resource.sampler);
        }

        Handle<GpuResource> newTexture (
            RenderContext& renderContext,
            std::filesystem::path path
        ) {
            Handle<GpuResource> handle{
                .id = m_nextID++
            };

            m_resources.emplace(
                handle.id,
                GpuResource {
                    .type = GpuResource::Type::Texture,
                    .texture = m_textureManager.newTexture(
                        renderContext.device,
                        renderContext.queue,
                        path
                    )
                }
            );

            return handle;
        }


        ImageTexture& getTexture(Handle<GpuResource> handle) {
            GpuResource resource = m_resources.at(handle.id);

            ASSERT(resource.type == GpuResource::Type::Texture, "Given handle was not buffer handle.", 0);

            return m_textureManager.getTexture(resource.texture);
        }

        void bindResource(std::vector<WGPUBindGroupEntry>& entries, Handle<GpuResource> handle) {
            auto resource = m_resources.at(handle.id);

            switch (resource.type) {
                case GpuResource::Type::Atlas:{
                    TextureAtlas& atlas = m_atlasManager.getAtlas(resource.atlas);


                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .offset = 0,
                        .textureView = atlas.getView()
                    });

                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .buffer = atlas.getBuffer().getRawHandle(),
                        .offset = 0,
                        .size = atlas.getBuffer().getByteSize()
                    });

                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .buffer = atlas.getPagesPerRowBuffer().getRawHandle(),
                        .offset = 0,
                        .size = atlas.getPagesPerRowBuffer().getByteSize()
                    });
                }
                break;
                case GpuResource::Type::Buffer: {
                    Buffer& buffer = m_bufferManager.getBuffer(resource.buffer);

                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .buffer = buffer.getRawHandle(),
                        .offset = 0,
                        .size = buffer.getByteSize()
                    });
                }
                break;
                case GpuResource::Type::MeshCollection: {
                    MeshCollection& meshCollection = m_meshManager.getCollection(resource.meshCollection);


                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .buffer = meshCollection.chunkBuffer().getRawHandle(),
                        .offset = 0,
                        .size = meshCollection.chunkBuffer().getByteSize()
                    });

                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .buffer = meshCollection.instanceBuffer().getRawHandle(),
                        .offset = 0,
                        .size = meshCollection.instanceBuffer().getByteSize()
                    });

                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .buffer = meshCollection.meshMapBuffer().getRawHandle(),
                        .offset = 0,
                        .size = meshCollection.meshMapBuffer().getByteSize()
                    });

                }
                break;
                case GpuResource::Type::Sampler: {
                    Sampler& sampler = m_samplerManager.getSampler(resource.sampler);

                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .sampler = sampler.getRawHandle(),
                    });
                }
                break;
                case GpuResource::Type::Texture: {
                    ImageTexture& texture = m_textureManager.getTexture(resource.texture);

                    entries.emplace_back(WGPUBindGroupEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .offset = 0,
                        .textureView = texture.getTextureView()
                    });
                }
                break;
            }

        }

        void getLayout(std::vector<wgpu::BindGroupLayoutEntry>& entries, Handle<GpuResource> handle) {
            auto resource = m_resources.at(handle.id);

            switch (resource.type) {
                case GpuResource::Type::Atlas:{
                    TextureAtlas& atlas = m_atlasManager.getAtlas(resource.atlas);

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = atlas.getStageVisibility(),
                        .texture = atlas.getBindingLayout()
                    });

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = atlas.getBuffer().getStageVisibility(),
                        .buffer = atlas.getBuffer().getBindingLayout()
                    });

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = atlas.getPagesPerRowBuffer().getStageVisibility(),
                        .buffer = atlas.getPagesPerRowBuffer().getBindingLayout()
                    });
                }
                break;
                case GpuResource::Type::Buffer: {
                    Buffer& buffer = m_bufferManager.getBuffer(resource.buffer);

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = buffer.getStageVisibility(),
                        .buffer = buffer.getBindingLayout()
                    });
                }
                break;
                case GpuResource::Type::MeshCollection: {
                    MeshCollection& meshCollection = m_meshManager.getCollection(resource.meshCollection);

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = meshCollection.chunkBuffer().getStageVisibility(),
                        .buffer = meshCollection.chunkBuffer().getBindingLayout()
                    });

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = meshCollection.instanceBuffer().getStageVisibility(),
                        .buffer = meshCollection.instanceBuffer().getBindingLayout()
                    });

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = meshCollection.meshMapBuffer().getStageVisibility(),
                        .buffer = meshCollection.meshMapBuffer().getBindingLayout()
                    });

                }
                break;
                case GpuResource::Type::Sampler: {
                    Sampler& sampler = m_samplerManager.getSampler(resource.sampler);

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = sampler.getStageVisibility(),
                        .sampler = sampler.getBindingLayout()
                    });
                }
                break;
                case GpuResource::Type::Texture: {
                    ImageTexture& texture = m_textureManager.getTexture(resource.texture);

                    entries.emplace_back(WGPUBindGroupLayoutEntry {
                        .nextInChain = nullptr,
                        .binding = (uint32_t) entries.size(),
                        .visibility = texture.getStageVisibility(),
                        .texture = texture.getBindingLayout()
                    });
                }
                break;
            }
        }

    private:
        GpuResourceID m_nextID = 0;

        std::unordered_map<GpuResourceID, GpuResource> m_resources;
        std::unordered_map<GpuResourceID, wgpu::BindGroupLayoutEntry> m_resourceBindings;

        AtlasManager m_atlasManager;
        BufferManager m_bufferManager;
        MeshManager m_meshManager;
        SamplerManager m_samplerManager;
        TextureManager m_textureManager;
    };

}
