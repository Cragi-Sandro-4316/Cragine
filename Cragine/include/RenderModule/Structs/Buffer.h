#pragma once

#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/BufferView.h"
#include "utils/Logger.h"
#include <thread>
#include <webgpu/webgpu.hpp>


namespace crg::renderer {

    class IBuffer {

    };

    enum BufferType : uint32_t {
        Storage,
        StorageReadable,
        Uniform,
        Debug
    };

    template<wgpu::BufferBindingType BindingType, typename T>
    class Buffer : public IBuffer {
    public:
        static constexpr wgpu::BufferBindingType bindingType = BindingType;
        using Type = T;

        Buffer(
            RenderContext& renderContext,
            wgpu::BufferUsage usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage
        ) :
        m_device(renderContext.device),
        m_queue(renderContext.queue),
        m_usage(usage) {
            WGPUBufferDescriptor desc{
                .label = wgpu::StringView(""),
                .usage = m_usage,
                .size = sizeof(T),
                .mappedAtCreation = false
            };

            m_buffer = m_device.createBuffer(desc);
        }


        BufferView<T> getBufferView() {
            if (
                !(m_usage & wgpu::BufferUsage::MapRead) &&
                !(m_usage & wgpu::BufferUsage::MapWrite)
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
                sizeof(T),
                callbackInfo
            );

            while (!mappingDone) {
                wgpu::SubmissionIndex sub{};

                m_device.poll(false, &sub);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            T* data = (T*)m_buffer.getMappedRange(
                0,
                sizeof(T)
            );

            return BufferView<T>(data, m_buffer, sizeof(T));
        }


        wgpu::Buffer& getRawBuffer() {
            return m_buffer;
        }

        void write(T data) {
            m_queue.writeBuffer(
                m_buffer,
                0,
                &data,
                sizeof(T)
            );
        }

    private:

        wgpu::Buffer m_buffer;

        wgpu::BufferUsage m_usage;

        wgpu::Queue m_queue;

        wgpu::Device m_device;
    };


}
