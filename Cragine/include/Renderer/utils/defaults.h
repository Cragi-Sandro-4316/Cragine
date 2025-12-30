#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace crg::renderer::utils {

    VkImageCreateInfo imageCreateInfo(
        VkFormat format,
        VkImageUsageFlags usageFlags,
        VkExtent3D extent
    );

    VkImageViewCreateInfo imageViewCreateInfo(
        VkFormat format,
        VkImage image,
        VkImageAspectFlags aspectFlags
    );

    VkCommandPoolCreateInfo commandPoolCreateInfo(
        uint32_t queueFamilyIndex,
        VkCommandPoolCreateFlags flags
    );

    VkCommandBufferAllocateInfo commandBufferAllocateInfo(
        VkCommandPool pool,
        uint32_t count
    );

    VkFenceCreateInfo fenceCreateInfo(
        VkFenceCreateFlags flags
    );

    VkSemaphoreCreateInfo semaphoreCreateInfo(
        VkSemaphoreCreateFlags flags
    );

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags);

    VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

    VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd);

    VkSubmitInfo2 submitInfo(
        VkCommandBufferSubmitInfo* cmd,
        VkSemaphoreSubmitInfo* signalSemaphoreInfo,
        VkSemaphoreSubmitInfo* waitSemaphoreInfo
    );
}
