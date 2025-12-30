#pragma once

#include "vulkan/vulkan.h"
#include <vulkan/vulkan_core.h>

namespace crg::renderer::imageOp {
    void transitionImage(
       VkCommandBuffer cmd,
       VkImage image,
       VkImageLayout currentLayout,
       VkImageLayout newLayout
    );

    VkImageSubresourceRange imageSubresourceRange(
        VkImageAspectFlags aspectMask
    );
}
