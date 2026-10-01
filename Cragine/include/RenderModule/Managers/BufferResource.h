#pragma once
#include "RenderModule/Managers/GpuResource.h"
#include "RenderModule/RenderContext.h"
#include "RenderModule/Structs/Buffer.h"
#include "Ecs/Handle.h"
#include <memory>
#include <unordered_map>


namespace crg::renderer {

    template<>
    class GpuResource<IBuffer> {
    public:

        template<wgpu::BufferBindingType BindingType, typename T>
        Handle<Buffer<BindingType, T>> add(
            RenderContext& renderContext,
            wgpu::BufferUsage usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage
        ) {

            Handle<Buffer<BindingType, T>> handle {
                .id = m_nextID++
            };

            m_buffers.emplace(
                handle.id,
                std::unique_ptr<IBuffer>(std::make_unique<Buffer<BindingType, T>>(
                    renderContext,
                    usage
                ))
            );
            return handle;
        }


        template<wgpu::BufferBindingType BindingType, typename T>
        Buffer<BindingType, T>& get(Handle<Buffer<BindingType, T>> handle) {
            IBuffer x = *m_buffers.at(handle.id);

            return static_cast<Buffer<BindingType, T>&>(*m_buffers.at(handle.id).get());
        }


    private:
        size_t m_nextID = 0;

        std::unordered_map<size_t, std::unique_ptr<IBuffer>> m_buffers;

    };

}
