#pragma once

#include "RenderModule/Structs/Sampler.h"
#include "utils/Logger.h"
#include "RenderModule/Handles.h"

namespace crg::renderer {


    class SamplerManager {
    public:

        Handle<TextureSampler> newSampler(wgpu::Device& device, wgpu::Queue& queue) {

            Handle<TextureSampler> handle {
                .id = m_currentID
            };

            m_samplers.insert({m_currentID, TextureSampler(device, queue)});

            m_currentID++;

            return handle;
        }


        TextureSampler* getSamplerPtr(Handle<TextureSampler> handle) {
            auto it = m_samplers.find(handle.id);

            if (it == m_samplers.end()) {
                LOG_CORE_ERROR("Gpu getBuffer error: given handle is invalid");
                return nullptr;
            }

            return &it->second;
        }

        inline bool validateHandle(Handle<TextureSampler> handle) {
            return m_samplers.contains(handle.id);
        }

        void deleteSampler(Handle<TextureSampler> handle) {
            if (!validateHandle(handle)) {
                LOG_CORE_WARNING("Sampler deletion error: given handle is invalid");
                return;
            }

            m_samplers.erase(handle.id);
        }

    private:
        size_t m_currentID = 0;

        std::unordered_map<size_t, TextureSampler> m_samplers;
    };



}
