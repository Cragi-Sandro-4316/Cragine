#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <type_traits>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>
#include <boost/pfr/core.hpp>

#include "RenderModule/Managers/BufferResource.h"
#include "RenderModule/Managers/GpuResource.h"
#include "RenderModule/RenderContext.h"
#include "utils/Logger.h"
#include "utils/Assert.h"

#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Structs/ImageTexture.h"
#include "RenderModule/Structs/MeshCollection.h"
#include "RenderModule/Structs/Sampler.h"
#include "RenderModule/Structs/TextureAtlas.h"


namespace crg::renderer {

    struct IMaterial {

        explicit IMaterial(RenderContext& ctx) :
        m_meshCollection(ctx) {}

        uint32_t getVertexCount() {
            return m_meshCollection.vertexCount();
        }

        virtual wgpu::BindGroup getBindGroup(
            RenderContext& ctx,
            GpuResource<IBuffer>& buffermanager,
            GpuResource<ImageTexture>& textureManager,
            GpuResource<Sampler>& samplerManager,
            GpuResource<TextureAtlas>& atlasManager
        ) = 0;

        MeshCollection m_meshCollection;

        wgpu::BindGroupLayout m_bindingLayout;
        wgpu::BindGroup m_bindGroup;
        wgpu::RenderPipeline m_pipeline;
        wgpu::ShaderModule m_shaderModule;
    };



    template<typename MaterialDef>
    struct Material : IMaterial {
    public:
        Material(
            const std::filesystem::path& path,
            RenderContext& renderContext,
            MaterialDef def
        ) :
        IMaterial(renderContext),
        m_materialDef(def) {
            std::vector<WGPUBindGroupLayoutEntry> layoutEntries;

            WGPUBufferBindingLayout buffer{
                .nextInChain = nullptr,
                .type = MeshCollection::bindingType,
                .hasDynamicOffset = false,
                .minBindingSize = sizeof(typename MeshCollection::Type)
            };

            layoutEntries.emplace_back(WGPUBindGroupLayoutEntry {
                .nextInChain = nullptr,
                .binding = (uint32_t)layoutEntries.size(),
                .visibility = wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex,
                .buffer = buffer
            });

            LOG_CORE_TRACE("Added MeshCollection");

            boost::pfr::for_each_field(
                m_materialDef,
                [&](auto& field) {
                    using field_t = typename std::remove_cvref_t<decltype(field)>::Type;

                    LOG_CORE_INFO("type name: {}", typeid(field_t).name());
                    constexpr bool meshCollection = !std::is_same_v<MeshCollection, field_t>;
                    ASSERT(meshCollection, "Material {} cannot have user-defined mesh collection", path.c_str());

                    fillLayoutEntries<field_t>(layoutEntries, field);
                }
            );

            wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
            bindGroupLayoutDesc.nextInChain = nullptr;
            bindGroupLayoutDesc.label = wgpu::StringView(path.c_str());
            bindGroupLayoutDesc.entryCount = layoutEntries.size();
            bindGroupLayoutDesc.entries = layoutEntries.data();

            m_bindingLayout = renderContext.device.createBindGroupLayout(bindGroupLayoutDesc);

            std::ifstream file(path);

            if (!file.is_open()) {
                LOG_CORE_ERROR("Failed to open file {}", path.c_str());
                return;
            }
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            std::string shaderSource(size, ' ');
            file.seekg(0);
            file.read(shaderSource.data(), size);

            // Shader module code:
            wgpu::ShaderSourceWGSL shaderCodeDesc{};
            shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
            shaderCodeDesc.code = wgpu::StringView(shaderSource.c_str());

            wgpu::ShaderModuleDescriptor shaderDesc{};
            shaderDesc.nextInChain = &shaderCodeDesc.chain;

            m_shaderModule = renderContext.device.createShaderModule(shaderDesc);

            // Pipeline code:
            wgpu::PipelineLayoutDescriptor pipelineLayoutDesc{};
            pipelineLayoutDesc.bindGroupLayoutCount = 1;
            pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*) &m_bindingLayout;
            pipelineLayoutDesc.label = wgpu::StringView(path.generic_string().append(" layout"));
            pipelineLayoutDesc.nextInChain = nullptr;

            auto pipelineLayout = renderContext.device.createPipelineLayout(pipelineLayoutDesc);

            wgpu::RenderPipelineDescriptor pipelineDesc{};
            pipelineDesc.label = wgpu::StringView(path.generic_string().append(" pipeline"));
            pipelineDesc.layout = pipelineLayout;
            pipelineDesc.depthStencil = &renderContext.depthStencilState;

            // Pipeline states
            wgpu::VertexState vertState{};
            vertState.nextInChain = nullptr;
            vertState.module = m_shaderModule;
            vertState.entryPoint = wgpu::StringView("vs_main");
            vertState.bufferCount = 0;
            vertState.buffers = nullptr;
            vertState.constantCount = 0;
            vertState.constants = nullptr;

            wgpu::BlendState blendState{};
            blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
            blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
            blendState.color.operation = wgpu::BlendOperation::Add;
            blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
            blendState.alpha.dstFactor = wgpu::BlendFactor::One;
            blendState.alpha.operation = wgpu::BlendOperation::Add;

            std::vector<wgpu::ColorTargetState> colorTargetState{};
            colorTargetState.emplace_back();
            colorTargetState[0].format = renderContext.config.format;
            colorTargetState[0].writeMask = wgpu::ColorWriteMask::All;
            colorTargetState[0].blend = &blendState;

            wgpu::FragmentState fragState{};
            fragState.nextInChain = nullptr;
            fragState.module = m_shaderModule;
            fragState.entryPoint = wgpu::StringView("fs_main");
            fragState.constantCount = 0;
            fragState.constants = nullptr;
            fragState.targetCount = colorTargetState.size();
            fragState.targets = colorTargetState.data();

            wgpu::PrimitiveState primitiveState{};
            primitiveState.nextInChain = nullptr;
            primitiveState.topology = wgpu::PrimitiveTopology::TriangleList;
            primitiveState.frontFace = wgpu::FrontFace::CCW;
            primitiveState.cullMode = wgpu::CullMode::None;
            primitiveState.unclippedDepth = false;
            primitiveState.stripIndexFormat = wgpu::IndexFormat::Undefined;

            wgpu::MultisampleState multiSampleState{};
            multiSampleState.count = 1;
            multiSampleState.mask = !0;
            multiSampleState.alphaToCoverageEnabled = false;

            pipelineDesc.vertex = vertState;
            pipelineDesc.fragment = &fragState;

            pipelineDesc.primitive = primitiveState;
            pipelineDesc.multisample = multiSampleState;

            m_pipeline = renderContext.device.createRenderPipeline(pipelineDesc);
        }


        virtual wgpu::BindGroup getBindGroup(
            RenderContext& renderContext,
            GpuResource<IBuffer>& buffermanager,
            GpuResource<ImageTexture>& textureManager,
            GpuResource<Sampler>& samplerManager,
            GpuResource<TextureAtlas>& atlasManager
        ) override {

            static bool bound = false;

            if (!bound) {
                std::vector<WGPUBindGroupEntry> entries;

                entries.emplace_back(WGPUBindGroupEntry {
                    .nextInChain = nullptr,
                    .binding = (uint32_t)entries.size(),
                    .buffer = m_meshCollection.getRawBuffer(),
                    .offset = 0,
                    .size = sizeof(typename MeshCollection::Type)
                });

                boost::pfr::for_each_field(
                    m_materialDef,
                    [&](auto& field) {
                        using field_t = typename std::remove_cvref_t<decltype(field)>::Type;

                        bind<field_t>(
                            entries,
                            field,
                            renderContext,
                            buffermanager,
                            textureManager,
                            samplerManager,
                            atlasManager
                        );
                    }
                );


                WGPUBindGroupDescriptor desc {};
                desc.nextInChain = nullptr,
                desc.layout = m_bindingLayout,
                desc.label = wgpu::StringView(""),
                desc.entryCount = entries.size(),
                desc.entries = entries.data(),

                m_bindGroup = renderContext.device.createBindGroup(desc);
                bound = true;
            }

            return m_bindGroup;
        }


        MaterialDef m_materialDef;

    private:

        template<typename FieldType>
        void bind(
            std::vector<WGPUBindGroupEntry>& entries,
            auto& field,
            RenderContext& renderContext,
            GpuResource<IBuffer>& buffermanager,
            GpuResource<ImageTexture>& textureManager,
            GpuResource<Sampler>& samplerManager,
            GpuResource<TextureAtlas>& atlasManager
        ) {
            if constexpr (std::is_base_of_v<IBuffer, FieldType>) {
                auto buffer = buffermanager.get(field.handle);

                entries.emplace_back(WGPUBindGroupEntry{
                    .nextInChain = nullptr,
                    .binding = (uint32_t)entries.size(),
                    .buffer = buffer.getRawBuffer(),
                    .offset = 0,
                    .size = sizeof(typename FieldType::Type)
                });
                LOG_CORE_TRACE("Bound buffer {} to material", field.handle.id);
            }
            else if constexpr (std::is_same_v<ImageTexture, FieldType>) {
                auto texture = textureManager.get(field.handle);

                entries.emplace_back(WGPUBindGroupEntry{
                    .nextInChain = nullptr,
                    .binding = (uint32_t)entries.size(),
                    .offset = 0,
                    .textureView = texture.getTextureView(),
                });
                LOG_CORE_TRACE("Bound texture {} to material", field.handle.id);
            }
            else if constexpr (std::is_same_v<Sampler, FieldType>) {
                auto sampler = samplerManager.get(field.handle);

                entries.emplace_back(WGPUBindGroupEntry{
                    .nextInChain = nullptr,
                    .binding = (uint32_t)entries.size(),
                    .sampler = sampler.getRawHandle(),
                });
                LOG_CORE_TRACE("Bound texture {} to material", field.handle.id);
            }
            else if constexpr (std::is_same_v<TextureAtlas, FieldType>) {
                auto atlas = atlasManager.get(field.handle);

                entries.emplace_back(WGPUBindGroupEntry{
                    .nextInChain = nullptr,
                    .binding = (uint32_t)entries.size(),
                    .buffer = atlas.getBuffer().getRawBuffer(),
                    .offset = 0,
                    .size = sizeof(typename FieldType::BufferType)
                });

                entries.emplace_back(WGPUBindGroupEntry{
                    .nextInChain = nullptr,
                    .binding = (uint32_t)entries.size(),
                    .offset = 0,
                    .textureView = atlas.getView(),
                });
            }

        }



        template<typename FieldType>
        void fillLayoutEntries(std::vector<WGPUBindGroupLayoutEntry>& layoutEntries, auto& field) {
            if constexpr (std::is_base_of_v<IBuffer, FieldType>) {
                WGPUBufferBindingLayout buffer{
                    .nextInChain = nullptr,
                    .type = FieldType::bindingType,
                    .hasDynamicOffset = false,
                    .minBindingSize = sizeof(typename FieldType::Type)
                };

                layoutEntries.emplace_back(WGPUBindGroupLayoutEntry {
                    .nextInChain = nullptr,
                    .binding = (uint32_t)layoutEntries.size(),
                    .visibility = field.stage,
                    .buffer = buffer
                });

                LOG_CORE_TRACE("Added Buffer");
            }
            else if constexpr (std::is_same_v<ImageTexture, FieldType>) {

                layoutEntries.emplace_back(WGPUBindGroupLayoutEntry {
                    .nextInChain = nullptr,
                    .binding = (uint32_t)layoutEntries.size(),
                    .visibility = field.stage,
                    .texture = FieldType::bindingLayout
                });

                LOG_CORE_TRACE("Added Texture");
            }
            else if constexpr (std::is_same_v<Sampler, FieldType>) {

                layoutEntries.emplace_back(WGPUBindGroupLayoutEntry {
                    .nextInChain = nullptr,
                    .binding = (uint32_t)layoutEntries.size(),
                    .visibility = field.stage,
                    .sampler = WGPUSamplerBindingLayout {
                        .nextInChain = nullptr,
                        .type = wgpu::SamplerBindingType::Filtering
                    }
                });

                LOG_CORE_TRACE("Added Sampler");
            }
            else if constexpr (std::is_same_v<TextureAtlas, FieldType>) {
                layoutEntries.emplace_back(WGPUBindGroupLayoutEntry {
                    .nextInChain = nullptr,
                    .binding = (uint32_t)layoutEntries.size(),
                    .visibility = field.stage,
                    .buffer = WGPUBufferBindingLayout {
                        .nextInChain = nullptr,
                        .type = FieldType::bufferBindingType,
                        .hasDynamicOffset = false,
                        .minBindingSize = sizeof(typename FieldType::BufferType)
                    }
                });

                layoutEntries.emplace_back(WGPUBindGroupLayoutEntry {
                    .nextInChain = nullptr,
                    .binding = (uint32_t)layoutEntries.size(),
                    .visibility = field.stage,
                    .texture = FieldType::bindingLayout
                });
            }

        }

    };


}
