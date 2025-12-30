#include "defaults.h"
#include <vulkan/vulkan_core.h>

namespace crg::renderer::utils {

    VkImageCreateInfo imageCreateInfo(
        VkFormat format,
        VkImageUsageFlags usageFlags,
        VkExtent3D extent
    ) {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;

        info.imageType = VK_IMAGE_TYPE_2D;

        info.format = format;
        info.extent = extent;

        info.mipLevels = 1;
        info.arrayLayers = 1;

        //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
        info.samples = VK_SAMPLE_COUNT_1_BIT;

        //optimal tiling, which means the image is stored on the best gpu format
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usageFlags;

        return info;
    }

    VkImageViewCreateInfo imageViewCreateInfo(
        VkFormat format,
        VkImage image,
        VkImageAspectFlags aspectFlags
    ) {
        // build a image-view for the depth image to use for rendering
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = nullptr;

        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.image = image;
        info.format = format;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.subresourceRange.aspectMask = aspectFlags;

        return info;
    }

    VkCommandPoolCreateInfo commandPoolCreateInfo(
        uint32_t queueFamilyIndex,
        VkCommandPoolCreateFlags flags
    ) {
        VkCommandPoolCreateInfo info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .queueFamilyIndex = queueFamilyIndex
        };

        return info;
    }

    VkCommandBufferAllocateInfo commandBufferAllocateInfo(
        VkCommandPool pool,
        uint32_t count
    ) {
        VkCommandBufferAllocateInfo info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count
        };

        return info;
    }

    VkFenceCreateInfo fenceCreateInfo(
        VkFenceCreateFlags flags
    ) {
        VkFenceCreateInfo info {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags
        };

        return info;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo(
        VkSemaphoreCreateFlags flags
    ) {
        VkSemaphoreCreateInfo info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags
        };

        return info;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
        VkCommandBufferBeginInfo info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = flags,
            .pInheritanceInfo = nullptr
        };

        return info;
    }

    VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
        VkSemaphoreSubmitInfo submitInfo{};
    	submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    	submitInfo.pNext = nullptr;
    	submitInfo.semaphore = semaphore;
    	submitInfo.stageMask = stageMask;
    	submitInfo.deviceIndex = 0;
    	submitInfo.value = 1;

    	return submitInfo;

    }

    VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd) {
        VkCommandBufferSubmitInfo info{};
    	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    	info.pNext = nullptr;
    	info.commandBuffer = cmd;
    	info.deviceMask = 0;

    	return info;

    }

    VkSubmitInfo2 submitInfo(
        VkCommandBufferSubmitInfo* cmd,
        VkSemaphoreSubmitInfo* signalSemaphoreInfo,
        VkSemaphoreSubmitInfo* waitSemaphoreInfo
    ) {
        VkSubmitInfo2 info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        info.pNext = nullptr;

        info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
        info.pWaitSemaphoreInfos = waitSemaphoreInfo;

        info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
        info.pSignalSemaphoreInfos = signalSemaphoreInfo;

        info.commandBufferInfoCount = 1;
        info.pCommandBufferInfos = cmd;

        return info;
    }


}
