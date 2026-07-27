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
        Handle<Buffer> newBuffer(size_t size, wgpu::Device& device) {

            m_buffers.emplace(m_currentID, Buffer(3, BUFFER_TYPE(T), device));

            m_currentID++;

            Handle<Buffer> handle{ m_currentID };

            if (!m_typeMap.contains(typeid(T))) {
                m_typeMap[typeid(T)] = {};
            }

            m_typeMap[typeid(T)].emplace_back(handle);

            return handle;
        }


        Buffer* getBuffer(Handle<Buffer> handle) {
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

    private:
        size_t m_currentID = 0;

        std::unordered_map<size_t, Buffer> m_buffers;

        std::unordered_map<
            std::type_index,
            std::vector<Handle<Buffer>>
        > m_typeMap;
    };


}
