// #include "RenderModule/Managers/BindGroupManager.h"
// #include "RenderModule/GpuInterface.h"

// namespace crg::renderer {
//     wgpu::BindGroup& BindGroupManager::getBindGroup(
//         GpuInterface& gpuInterface,
//         MaterialID materialID,
//         Material& material
//     ) {
//         if (m_bindGroups.contains(materialID)) {
//             return m_bindGroups.at(materialID);
//         }

//         std::vector<WGPUBindGroupEntry> entries;
//         entries.reserve(material.m_resources.size());

//         for (auto& resource : material.m_resources) {
//             gpuInterface.getResourceManager().bindResource(entries, resource);
//         }


//         wgpu::BindGroupDescriptor bindGroupDesc{};
//         bindGroupDesc.nextInChain = nullptr;
//         bindGroupDesc.layout = material.m_bindingLayout;
//         bindGroupDesc.entryCount = entries.size();
//         bindGroupDesc.entries = entries.data();

//         m_bindGroups.emplace(
//             materialID,
//             gpuInterface.getRenderContext().device.createBindGroup(bindGroupDesc)
//         );

//         return m_bindGroups.at(materialID);
//     }

// }
