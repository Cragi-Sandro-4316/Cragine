#pragma once

#include "RenderModule/Structs/Buffer.h"
#include "RenderModule/Components/Camera.h"

namespace crg::renderer {

    class CameraBuffer {
    public:

        CameraBuffer(
            wgpu::Device& device,
            wgpu::Queue& queue
        ) :
        m_buffer(
            1,
            BUFFER_TYPE(CameraUniform),
            device,
            queue,
            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
            wgpu::BufferBindingType::Uniform,
            BufferType::Uniform
        ) {}


        Buffer& get() {
            return m_buffer;
        }

        void bindLayoutEntry(std::vector<WGPUBindGroupLayoutEntry>& entries) {
            entries.emplace_back(WGPUBindGroupLayoutEntry {
                .nextInChain = nullptr,
                .binding = (uint32_t) entries.size(),
                .visibility = m_buffer.getStageVisibility(),
                .buffer = m_buffer.getBindingLayout()
            });
        }

        void bindEntry(std::vector<WGPUBindGroupEntry>& entries) {

            entries.emplace_back(WGPUBindGroupEntry{
                .nextInChain = nullptr,
                .binding = (uint32_t)entries.size(),
                .buffer = m_buffer.getRawHandle(),
                .offset = 0,
                .size = m_buffer.getByteSize()
            });

        }

    private:
        Buffer m_buffer;

    };


}
