#pragma once
#include "RenderModule/Buffers/Buffer.h"
#include <typeindex>
#include <unordered_map>
#include "RenderModule/Handles.h"
#include "utils/Logger.h"


namespace crg::renderer {


    class BufferManager {
    public:

        template <typename T>
        Handle<Buffer> newBuffer(size_t size, wgpu::Device& device, wgpu::Queue& queue) {

            m_buffers.emplace(m_currentID, Buffer(3, BUFFER_TYPE(T), device, queue));

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
        void writeBuffer(Handle<Buffer> buffer, std::vector<T>& data, wgpu::Queue& queue) {
            auto it = m_buffers.find(buffer.id);
            if (it == m_buffers.end()) {
                LOG_CORE_WARNING("Buffer write: invalid handle");
                return;
            }

            it->second.writeBuffer(data, queue);
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
