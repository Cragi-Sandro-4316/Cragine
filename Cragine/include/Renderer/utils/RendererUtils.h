#pragma once

#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace crg::renderer::utils {
    vk::CommandPoolCreateInfo commandPoolCreateInfo(
        uint32_t queueFamilyIndex,
        vk::CommandPoolCreateFlags flags
    );

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
        vk::CommandPool pool,
        uint32_t count
    );


}
