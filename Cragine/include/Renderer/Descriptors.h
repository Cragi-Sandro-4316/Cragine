#pragma once

#include "vulkan/vulkan.hpp"

namespace crg::renderer {
    struct DescriptorLayoutBuilder {

        std::vector<vk::DescriptorSetLayoutBinding> m_bindings;

        void addBinding(uint32_t binding, vk::DescriptorType type);
        void clear();

        vk::DescriptorSetLayout build(
            vk::Device device,
            vk::ShaderStageFlags shaderStages,
            void* pNext = nullptr,
            vk::DescriptorSetLayoutCreateFlags flags = {}
        );
    };

    struct DescriptorAllocator {

        struct PoolSizeRatio {
            vk::DescriptorType type;
            float ratio;
        };

        vk::DescriptorPool pool;

        void initPool(
            vk::Device device,
            uint32_t maxSets,
            std::span<PoolSizeRatio> poolRatios
        );
        void clearDescriptors(vk::Device device);
        void destroyPool(vk::Device device);

        vk::DescriptorSet allocate(
            vk::Device device,
            vk::DescriptorSetLayout layout
        );

    };
}
