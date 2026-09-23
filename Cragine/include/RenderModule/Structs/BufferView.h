#pragma once
#include <cstddef>
#include <cstring>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    template<typename T>
    class BufferView {
    public:

        BufferView(
            void* data,
            wgpu::Buffer buffer,
            size_t size
        ) :
        m_data((T*)data),
        m_size(size),
        m_buffer(buffer) {}

        ~BufferView() {
            if (m_data)
                m_buffer.unmap();
        }

        BufferView(const BufferView&) = delete;
        BufferView& operator=(const BufferView&) = delete;

        BufferView(BufferView&& obj) = delete;
        BufferView& operator=(const BufferView&&) = delete;


        T& operator[](size_t index) {
            return m_data[index];
        }

        T* get() {
            return m_data;
        }

        // Inserts elements at the specified index
        void insert(T* values, size_t index, size_t count = 1) {
            std::memmove(
                m_data + index + count,
                m_data + index,
                (m_size - index) * sizeof(T)
            );

            std::memcpy(m_data + index, values, count * sizeof(T));
        }

        void erase(size_t index, size_t count = 1) {
            std::memmove(
                m_data + index,
                m_data + index + count,
                (m_size - index - count) * sizeof(T)
            );
        }

    private:
        T* m_data;
        const size_t m_size;
        wgpu::Buffer m_buffer;
    };


}
