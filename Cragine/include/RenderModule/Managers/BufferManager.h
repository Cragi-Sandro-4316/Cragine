#pragma once

#include "RenderModule/Structs/Buffer.h"

namespace crg::renderer {

    class BufferManager {
    public:

        template <typename T>
        Handle<Buffer> newBuffer(size_t size, wgpu::Device& device, wgpu::Queue& queue, BufferType bufferType) {

            wgpu::BufferBindingType bindingType{};
            wgpu::BufferUsage bufferUsage{};

            switch (bufferType){
                case BufferType::StorageReadable:
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead;
                break;
                case BufferType::Storage:
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
                break;
                case BufferType::Uniform:
                    bindingType = wgpu::BufferBindingType::Uniform;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
                break;
                case BufferType::Debug:
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapWrite;
                break;
                default:
                    LOG_CORE_WARNING("Gpu buffer creation: invalid buffer type, defaulting to storage.");
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
                break;
            }

            m_buffers.emplace(m_currentID, Buffer(size, BUFFER_TYPE(T), device, queue, bufferUsage, bindingType, bufferType));

            Handle<Buffer> handle{ m_currentID };

            m_currentID++;

            // TODO: is this actually needed?
            if (!m_typeMap.contains(typeid(T))) {
                m_typeMap[typeid(T)] = {};
            }

            m_typeMap[typeid(T)].emplace_back(handle);

            return handle;
        }


        Buffer& getBuffer(Handle<Buffer> handle) {
            return m_buffers.at(handle.id);
        }

        void deleteBuffer(Handle<Buffer> handle) {
            m_buffers.erase(handle.id);
        }

        template<typename T>
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data) {
            m_buffers.at(buffer.id).writeBuffer(data.data(), data.size());
        }

    private:
        size_t m_currentID = 0;

        std::unordered_map<size_t, Buffer> m_buffers;

        std::unordered_map<
            std::type_index,
            std::vector<Handle<Buffer>>
        > m_typeMap;
    };

}
