#pragma once

#include "RenderModule/Structs/BufferView.h"
#include "utils/Assert.h"
#include "utils/Logger.h"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <thread>
#include <typeindex>
#include <webgpu.h>
#include <webgpu/webgpu.hpp>

#define BUFFER_TYPE(type) (type*)nullptr

namespace crg::renderer {

    enum BufferType : uint32_t {
        Storage,
        StorageReadable,
        Uniform
    };

    class Buffer {
    public:

        struct DataTypeDesc {
            std::type_index typeID = typeid(void);
            size_t size;
            size_t align;
        };

        template<typename T>
        Buffer(
            size_t size,
            T* typePtr,
            wgpu::Device device,
            wgpu::Queue queue,
            wgpu::BufferUsage bufferUsage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
            wgpu::BufferBindingType bindingType = wgpu::BufferBindingType::Storage,
            BufferType bufferType = BufferType::Storage,
            wgpu::ShaderStage shaderStage = wgpu::ShaderStage::Vertex
        ):
        m_size(size),
        m_shaderStage(shaderStage),
        m_device(device),
        m_queue(queue),
        m_bufferType(bufferType),
        m_bufferUsage(bufferUsage),
        m_typeDesc(DataTypeDesc {
            .typeID = typeid(T),
            .size = sizeof(T),
            .align = alignof(T)
        }) {

            // LOG_CORE_INFO("size: {}", sizeof(T));

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

        wgpu::Buffer getRawHandle() const {
            return m_buffer;
        }

        wgpu::BufferBindingLayout getBindingLayout() const {
            return m_bindingLayout;
        }

        wgpu::ShaderStage getStageVisibility() const {
            return m_shaderStage;
        }

        size_t getByteSize() const {
            return m_size * m_typeDesc.size;
        }

        template<typename T>
        void writeBuffer(T* data, size_t count, size_t index = 0) {
            if (typeid(T) != m_typeDesc.typeID) {
                LOG_CORE_ERROR("GPU Buffer write: type mismatch");
                return;
            }

            size_t dataSize = count * m_typeDesc.size;
            size_t offset = index * m_typeDesc.size;

            if (dataSize > getByteSize()) {
                LOG_CORE_ERROR("GPU Buffer write: given vector and index fall out of bounds");
                return;
            }

            m_queue.writeBuffer(
                m_buffer,
                offset,
                data,
                dataSize
            );
        }

        template<typename T>
        void write(T& data, size_t index = 0) {

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

        size_t size() const {
            return m_size;
        }

        BufferType bufferType() const {
            return m_bufferType;
        }

        template<typename T>
        void read(std::vector<T>& buff) const {

            if (!(m_bufferUsage & wgpu::BufferUsage::MapRead)) {
                LOG_CORE_ERROR("Buffer not set for map reads");
                return;
            }

            bool mappingDone = false;

            wgpu::BufferMapCallbackInfo callbackInfo{};
            callbackInfo.nextInChain = nullptr;
            callbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
            callbackInfo.userdata1 = &mappingDone;
            callbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView, void* userData, void*) {
                bool* done = static_cast<bool*>(userData);
                *done = (status == wgpu::MapAsyncStatus::Success);
            };

            auto status = m_buffer.mapAsync(
                wgpu::MapMode::Read,
                0,
                getByteSize(),
                callbackInfo
            );

            while (!mappingDone) {
                wgpu::SubmissionIndex sub{};

                m_device.poll(false, &sub);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }


            T* mappedData = (T*)m_buffer.getMappedRange(
                0,
                getByteSize()
            );

            buff.resize(m_size);
            std::memcpy(buff.data(), mappedData, getByteSize());

            m_buffer.unmap();
        }

        template<typename T>
        BufferView<T> getBufferView() {
            if (
                !(m_bufferUsage & wgpu::BufferUsage::MapRead) &&
                !(m_bufferUsage & wgpu::BufferUsage::MapWrite)
            ) {
                LOG_CORE_ERROR("Buffer not set for buffer read and buffer write");
                return BufferView<T>(nullptr, {}, 0);
            }

            bool mappingDone = false;

            wgpu::BufferMapCallbackInfo callbackInfo{};
            callbackInfo.nextInChain = nullptr;
            callbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
            callbackInfo.userdata1 = &mappingDone;
            callbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView, void* userData, void*) {
                bool* done = static_cast<bool*>(userData);
                *done = (status == wgpu::MapAsyncStatus::Success);
            };

            auto status = m_buffer.mapAsync(
                wgpu::MapMode::Write,
                0,
                getByteSize(),
                callbackInfo
            );

            while (!mappingDone) {
                wgpu::SubmissionIndex sub{};

                m_device.poll(false, &sub);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            T* data = (T*)m_buffer.getMappedRange(
                0,
                getByteSize()
            );

            return BufferView<T>(data, m_buffer, m_size);
        }

    private:

        const wgpu::BufferUsage m_bufferUsage;

        const DataTypeDesc m_typeDesc;

        const BufferType m_bufferType;

        const size_t m_size;

        wgpu::Buffer m_buffer;

        wgpu::BufferBindingLayout m_bindingLayout;

        const wgpu::ShaderStage m_shaderStage;

        wgpu::Queue m_queue;

        wgpu::Device m_device;

    };

}
