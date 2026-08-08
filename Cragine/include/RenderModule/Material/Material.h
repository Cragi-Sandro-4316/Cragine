#pragma once
#include <webgpu/webgpu.hpp>
#include "RenderModule/Structs/Buffer.h"

namespace crg::renderer {

    struct Material {

        wgpu::RenderPipeline m_pipeline;
        std::vector<wgpu::ShaderModule> m_shaderModules;

        size_t m_totalVertexCount;

        wgpu::BindGroup m_binding;

        wgpu::BindGroupLayout m_bindingLayout;

        std::vector<Buffer*> m_buffers;
        // std::vector<Handle<Texture>> m_textures;
        // std::vector<Handle<TextureSampler>> m_samplers;

        void updateCounts() {
            m_totalVertexCount = 0;
            for (Buffer* buffer : m_buffers) {
                if (buffer->bufferType() == BufferType::Vertex) {
                    m_totalVertexCount += buffer->size();
                    break;
                }
            }
        }

        // TODO: build instance map method.

        void updateInstanceMap() {
            Buffer* instanceBuffer = nullptr;
            Buffer* indexBuffer = nullptr;
            Buffer* vertexBuffer = nullptr;

            for (Buffer* buffer : m_buffers) {
                if (buffer->bufferType() == BufferType::Index) {
                    indexBuffer = buffer;
                }
                else if (buffer->bufferType() == BufferType::Instance) {
                    instanceBuffer = buffer;
                }
                else if (buffer->bufferType() == BufferType::Vertex) {
                    vertexBuffer = buffer;
                }

                if (instanceBuffer && indexBuffer) {
                    break;
                }
            }

            // TODO: write the actual buffers

        }

    };


}
