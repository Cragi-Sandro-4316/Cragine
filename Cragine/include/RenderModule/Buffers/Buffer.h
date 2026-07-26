#pragma once

#include "utils/Assert.h"
#include <cstddef>
#include <webgpu/webgpu.hpp>

#define BUFFER_TYPE(type) (type*)nullptr


namespace crg::renderer {


    class Buffer {
    public:
        struct TypeDesc {
            size_t size;
            size_t align;
        };

        template<typename T>
        Buffer(
            size_t size,
            T* typePtr,
            wgpu::Device& device,
            wgpu::BufferUsage bufferUsage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst
        ):
        m_size(size) {
            m_typeDesc = {
                .size = sizeof(T),
                .align = alignof(T)
            };

            ASSERT(
                alignof(T) == 4 ||
                alignof(T) == 8 ||
                alignof(T) == 16,
                "Buffer struct '{}' does not follow wgpu alignment requirements", typeid(T).name()
            );

            wgpu::BufferDescriptor bufferDesc{};
            bufferDesc.mappedAtCreation = false;
            bufferDesc.size = m_typeDesc.size * m_size;
            bufferDesc.usage = bufferUsage;

            m_buffer = device.createBuffer(bufferDesc);
        }


    private:

        TypeDesc m_typeDesc;

        const size_t m_size;

        wgpu::Buffer m_buffer;


    };




}
