#include "Descriptors.h"
#include "vulkan/vulkan.hpp"
#include "Renderer.h"
#include <cstdint>

namespace crg::renderer {

    void DescriptorLayoutBuilder::addBinding(uint32_t binding, vk::DescriptorType type) {
        vk::DescriptorSetLayoutBinding newBind{};
        newBind.binding = binding;
        newBind.descriptorCount = 1;
        newBind.descriptorType = type;

        m_bindings.push_back(newBind);
    }

    void DescriptorLayoutBuilder::clear() {
        m_bindings.clear();
    }

    vk::DescriptorSetLayout DescriptorLayoutBuilder::build(
        vk::Device device,
        vk::ShaderStageFlags shaderStages,
        void* pNext,
        vk::DescriptorSetLayoutCreateFlags flags
    ) {
        for (auto& b : m_bindings) {
            b.stageFlags |= shaderStages;
        }

        vk::DescriptorSetLayoutCreateInfo info{};
        info.pNext = pNext;

        info.pBindings = m_bindings.data();
        info.bindingCount = (size_t)m_bindings.size();
        info.flags = flags;

        vk::DescriptorSetLayout set;
        VK_CHECK(device.createDescriptorSetLayout(&info, nullptr, &set));

        return set;
    }


    void DescriptorAllocator::initPool(
        vk::Device device,
        uint32_t maxSets,
        std::span<PoolSizeRatio> poolRatios
    ) {
        std::vector<vk::DescriptorPoolSize> poolSizes;
        for(PoolSizeRatio ratio : poolRatios) {
            poolSizes.emplace_back(vk::DescriptorPoolSize {
                ratio.type,
                uint32_t(ratio.ratio * maxSets)
            });
        }

        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.flags = vk::DescriptorPoolCreateFlags{};
        poolInfo.maxSets = maxSets;
        poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();

        VK_CHECK(device.createDescriptorPool(&poolInfo, nullptr, &pool));
    }

    void DescriptorAllocator::clearDescriptors(vk::Device device) {
        device.resetDescriptorPool(pool);
    }

    void DescriptorAllocator::destroyPool(vk::Device device) {
        device.destroyDescriptorPool(pool, nullptr);
    }

    vk::DescriptorSet DescriptorAllocator::allocate(
        vk::Device device,
        vk::DescriptorSetLayout layout
    ) {
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.pNext = nullptr;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        vk::DescriptorSet ds;
        VK_CHECK(device.allocateDescriptorSets(&allocInfo, &ds));

        return ds;
    }

}
