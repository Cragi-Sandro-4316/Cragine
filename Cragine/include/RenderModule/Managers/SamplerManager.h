#pragma once

#include "RenderModule/Structs/Sampler.h"
#include "utils/Logger.h"

namespace crg::renderer {


    class SamplerManager {
    public:

        Handle<Sampler> newSampler(wgpu::Device& device, wgpu::Queue& queue) {

            Handle<Sampler> handle {
                .id = m_currentID
            };

            m_samplers.insert({m_currentID, Sampler(device, queue)});

            m_currentID++;

            return handle;
        }


        Sampler& getSampler(Handle<Sampler> handle) {
            return m_samplers.at(handle.id);
        }

        inline bool validateHandle(Handle<Sampler> handle) {
            return m_samplers.contains(handle.id);
        }

        void deleteSampler(Handle<Sampler> handle) {
            if (!validateHandle(handle)) {
                LOG_CORE_WARNING("Sampler deletion error: given handle is invalid");
                return;
            }

            m_samplers.erase(handle.id);
        }

    private:
        size_t m_currentID = 0;

        std::unordered_map<size_t, Sampler> m_samplers;
    };



}
