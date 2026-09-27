#pragma once

#include "RenderModule/GpuInterface.h"
#include "RenderModule/GpuResourceManager.h"
#include "RenderModule/Managers/MaterialManager.h"
#include <unordered_map>
#include <webgpu/webgpu.hpp>

namespace crg::renderer {

    class BindGroupManager {
    public:

        wgpu::BindGroup& getBindGroup(
            GpuInterface& gpuInterface,
            MaterialID materialID,
            Material& material
        ) {
            if (m_bindGroups.contains(materialID)) {
                return m_bindGroups.at(materialID);
            }

            std::vector<WGPUBindGroupEntry> entries;
            entries.reserve(material.m_resources.size());

            for (auto& resource : material.m_resources) {
                gpuInterface.getResourceManager().bindResource(entries, resource);
            }


            wgpu::BindGroupDescriptor bindGroupDesc{};
            bindGroupDesc.nextInChain = nullptr;
            bindGroupDesc.layout = material.m_bindingLayout;
            bindGroupDesc.entryCount = entries.size();
            bindGroupDesc.entries = entries.data();

            m_bindGroups.emplace(
                materialID,
                gpuInterface.getRenderContext().device.createBindGroup(bindGroupDesc)
            );

            return m_bindGroups.at(materialID);
        }


    private:

        std::unordered_map<MaterialID, wgpu::BindGroup> m_bindGroups;


    };


}
