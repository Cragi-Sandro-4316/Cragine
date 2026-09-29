#pragma once

#include <unordered_map>

#include "Ecs/Ecs.h"
#include "RenderModule/Components/Camera.h"
#include "RenderModule/Structs/CameraBuffer.h"
#include "RenderModule/Handles.h"

namespace crg::renderer {

    class CameraManager {
    public:

        Handle<CameraBuffer> newCamera (
            wgpu::Device& device,
            wgpu::Queue& queue,
            Camera cameraData,
            Entity entity
        ) {
            m_buffers.insert({
                entity.id,
                CameraBuffer(device, queue)
            });

            m_buffers.at(entity.id).get().write(cameraData.getUniform());

            return Handle<CameraBuffer> {
                .id = entity.id
            };
        }


        CameraBuffer& getCamera(Handle<CameraBuffer> handle) {
            return m_buffers.at(handle.id);
        }

        Handle<CameraBuffer> getCameraHandle(Entity entity) {
            ASSERT(m_buffers.contains(entity.id), "getCameraHandle: Requested entity is not a camera", 0);
            return Handle<CameraBuffer> {
                .id = entity.id
            };
        }

        void deleteCamera(Handle<CameraBuffer> handle) {
            m_buffers.erase(handle.id);
        }

    private:
        size_t m_currentID = 0;

        std::unordered_map<EntityId, CameraBuffer> m_buffers;
    };



}
