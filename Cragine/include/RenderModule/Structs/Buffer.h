#pragma once

#include "utils/Assert.h"
#include "utils/Logger.h"
#include <cstddef>
#include <cstdint>
#include <typeindex>
#include <webgpu/webgpu.hpp>

#define BUFFER_TYPE(type) (type*)nullptr

namespace crg::renderer {


    enum BufferType : uint32_t {
        Storage,
        Uniform
    };

    class Buffer {
    public:
        struct TypeDesc {
            std::type_index typeID = typeid(void);
            size_t size;
            size_t align;
        };

        template<typename T>
        Buffer(
            size_t size,
            T* typePtr,
            wgpu::Device& device,
            wgpu::Queue& queue,
            wgpu::BufferBindingType bindingType,
            wgpu::BufferUsage bufferUsage,
            wgpu::ShaderStage shaderStage = wgpu::ShaderStage::Vertex
        ):
        m_size(size),
        m_shaderStage(shaderStage),
        m_queue(queue) {
            m_typeDesc = {
                .typeID = typeid(T),
                .size = sizeof(T),
                .align = alignof(T)
            };

            LOG_CORE_INFO("size: {}", sizeof(T));

            ASSERT(     // TODO: Check this assert and make it work
                (sizeof(T) % 16 == 0) ||
                (sizeof(T) % 4 == 0 && sizeof(T) < 12),
                "Buffer struct '{}' does not follow wgpu alignment requirements. alignment: {}", typeid(T).name(), alignof(T)
            );

            wgpu::BufferDescriptor bufferDesc{};
            bufferDesc.label = wgpu::StringView("Buffer");
            bufferDesc.mappedAtCreation = false;
            bufferDesc.size = m_typeDesc.size * size;
            bufferDesc.usage = bufferUsage;

            m_buffer = device.createBuffer(bufferDesc);

            m_bindingLayout.nextInChain = nullptr;
            m_bindingLayout.type = bindingType;
            m_bindingLayout.hasDynamicOffset = false;
            m_bindingLayout.minBindingSize = m_typeDesc.size * m_size;

        }


        wgpu::Buffer getRawHandle() {
            return m_buffer;
        }

        wgpu::BufferBindingLayout getBindingLayout() {
            return m_bindingLayout;
        }

        wgpu::ShaderStage getStageVisibility() {
            return m_shaderStage;
        }

        size_t getByteSize() {
            return m_size * m_typeDesc.size;
        }

        template<typename T>
        void writeBuffer(std::vector<T>& data) {
            if (typeid(T) != m_typeDesc.typeID) {
                LOG_CORE_ERROR("GPU BUFFER: write type mismatch");
                return;
            }


            void* daata = data.data();
            size_t x = data.size();


            m_queue.writeBuffer(
                m_buffer,
                0,
                data.data(),
                data.size() * m_typeDesc.size
            );
        }


        template<typename T>
        void write(T& data, size_t index) {
            if (typeid(T) != m_typeDesc.typeID) {
                LOG_CORE_ERROR("GPU BUFFER WRITE: type mismatch");
                return;
            }
            if (index > m_size) {
                LOG_CORE_ERROR("GPU BUFFER WRITE: index out of bounds");
                return;
            }

            m_queue.writeBuffer(
                m_buffer,
                m_typeDesc.size * index,
                &data,
                m_typeDesc.size
            );
        }



    private:

        TypeDesc m_typeDesc;

        const size_t m_size;

        wgpu::Buffer m_buffer;

        wgpu::BufferBindingLayout m_bindingLayout;

        wgpu::ShaderStage m_shaderStage;

        wgpu::Queue& m_queue;
    };




}
