#pragma once

#include "Renderer/utils/helpers.h"
#include <cstdint>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    template<typename T>
    class Uniform {
    public:
        Uniform(
            wgpu::Device& device,
            wgpu::Queue& queue,
            uint32_t maxInstances = 1
        ) :
        m_device(device),
        m_queue(queue),
        m_maxInstances(maxInstances) {
            m_stride = helpers::ceilToNextMultiple(sizeof(T), getMinAlignment());

            // Create buffer
            wgpu::BufferDescriptor desc{};
            desc.size = m_stride * m_maxInstances;
            desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
            desc.mappedAtCreation = false;
            m_buffer = m_device.createBuffer(desc);
        }

        void update(const T& value, uint32_t instanceIndex = 0) {
            if (instanceIndex <= m_maxInstances) {
                uint64_t offset = instanceIndex * m_stride;
                m_queue.writeBuffer(m_buffer, offset, &value, sizeof(T));
            }
        }

        template<typename D>
        void writeField(const D* fieldPtr, size_t fieldOffset = 0) {
            m_queue.writeBuffer(m_buffer, fieldOffset, fieldPtr, sizeof(D));
        }

        wgpu::Buffer getBuffer() const { return m_buffer; }
        uint32_t getStride() const { return m_stride; }

    private:

        uint32_t getMinAlignment() {
            wgpu::Limits limits{};
            m_device.getLimits(&limits);
            return static_cast<uint32_t>(limits.minUniformBufferOffsetAlignment);
        }


    private:
        wgpu::Device& m_device;
        wgpu::Queue& m_queue;
        wgpu::Buffer m_buffer;
        uint32_t m_stride;
        uint32_t m_maxInstances;
    };
}
