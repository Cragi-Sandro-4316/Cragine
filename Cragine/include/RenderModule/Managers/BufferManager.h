#pragma once
#include "AssetManager/AssetManager.h"
#include "RenderModule/Structs/Buffer.h"
#include <typeindex>
#include <unordered_map>
#include <webgpu/webgpu.hpp>
#include "RenderModule/Handles.h"
#include "utils/Logger.h"


namespace crg::renderer {


    class BufferManager {
    public:

        template <typename T>
        Handle<Buffer> newBuffer(size_t size, wgpu::Device& device, wgpu::Queue& queue, BufferType bufferType) {

            wgpu::BufferBindingType bindingType{};
            wgpu::BufferUsage bufferUsage{};

            switch (bufferType) {
                case BufferType::Vertex:
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
                break;
                case BufferType::Index:
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
                break;
                case BufferType::Instance:
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
                break;
                case BufferType::Storage:
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
                break;
                case BufferType::Uniform:
                    bindingType = wgpu::BufferBindingType::Uniform;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
                break;
                default:
                    LOG_CORE_WARNING("Gpu buffer creation: invalid buffer type, defaulting to storage.");
                    bindingType = wgpu::BufferBindingType::Storage;
                    bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
                break;
            }

            m_buffers.emplace(m_currentID, Buffer(size, BUFFER_TYPE(T), device, queue, bindingType, bufferUsage, bufferType));

            Handle<Buffer> handle{ m_currentID };

            m_currentID++;

            if (!m_typeMap.contains(typeid(T))) {
                m_typeMap[typeid(T)] = {};
            }

            m_typeMap[typeid(T)].emplace_back(handle);

            return handle;
        }


        Buffer* getBufferPtr(Handle<Buffer> handle) {
            auto it = m_buffers.find(handle.id);

            if (it == m_buffers.end()) {
                LOG_CORE_ERROR("Gpu getBuffer error: given handle is invalid");
                return nullptr;
            }

            return &it->second;
        }

        inline bool validateHandle(Handle<Buffer> handle) {
            return m_buffers.contains(handle.id);
        }

        void deleteBuffer(Handle<Buffer> handle) {
            if (!validateHandle(handle)) {
                LOG_CORE_WARNING("Buffer deletion error: given handle is invalid");
                return;
            }

            m_buffers.erase(handle.id);
        }

        template<typename T>
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data) {
            auto it = m_buffers.find(buffer.id);
            if (it == m_buffers.end()) {
                LOG_CORE_WARNING("Buffer write: invalid handle");
                return;
            }

            it->second.writeBuffer(data);
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
