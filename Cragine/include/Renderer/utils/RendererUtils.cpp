#include "RendererUtils.h"
#include "vulkan/vulkan.hpp"

namespace crg::renderer::utils {
    vk::CommandPoolCreateInfo commandPoolCreateInfo(
        uint32_t queueFamilyIndex,
        vk::CommandPoolCreateFlags flags
    ) {
        vk::CommandPoolCreateInfo info{};
        info.pNext = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;

        return info;
    }

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
        vk::CommandPool pool,
        uint32_t count
    ) {
        vk::CommandBufferAllocateInfo info{};
        info.pNext = nullptr;

        info.commandPool = pool;
        info.commandBufferCount = count;
        info.level = vk::CommandBufferLevel::ePrimary;
        return info;
    }
}
