#pragma once

#include "RenderModule/Structs/Buffer.h"
#include <unordered_map>
#include <Ecs/Handle.h>
#include <utility>

namespace crg::renderer {

    template<typename Resource>
    class GpuResource {
    public:

        template<typename... Args>
        Handle<Resource> add(Args&&... args) {
            Handle<Resource> handle {
                .id = m_nextID++
            };

            m_resources.emplace(
                handle.id,
                std::forward<Args>(args)...
            );
            return handle;
        }

        Resource& get(Handle<Resource> handle) {
            return m_resources.at(handle.id);
        }


    private:

        size_t m_nextID = 0;

        std::unordered_map<size_t, Resource> m_resources;
    };

    template<> class GpuResource<IBuffer>;
}
