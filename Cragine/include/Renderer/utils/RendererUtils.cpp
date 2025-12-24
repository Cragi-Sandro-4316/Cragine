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

    vk::FenceCreateInfo fenceCreateInfo(
        vk::FenceCreateFlags flags
    ) {
        vk::FenceCreateInfo info{};
        info.pNext = nullptr;

        info.flags = flags;

        return info;
    }

    vk::SemaphoreCreateInfo semaphoreCreateInfo(
        vk::SemaphoreCreateFlags flags
    ) {
        vk::SemaphoreCreateInfo info{};

        info.pNext = nullptr;
        info.flags = flags;
        return info;
    }

    vk::CommandBufferBeginInfo commandBufferBeginInfo(
        vk::CommandBufferUsageFlags flags
    ) {
        vk::CommandBufferBeginInfo info{};
        info.pNext = nullptr;

        info.pInheritanceInfo = nullptr;
        info.flags = flags;
        return info;
    }

    void transitionImage(
        vk::CommandBuffer cmd,
        vk::Image image,
        vk::ImageLayout currentlayout,
        vk::ImageLayout newLayout
    ) {
        vk::ImageMemoryBarrier2 imageBarrier{};
        imageBarrier.pNext = nullptr;

        imageBarrier.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
        imageBarrier.srcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
        imageBarrier.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands;
        imageBarrier.dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;

        imageBarrier.oldLayout = currentlayout;
        imageBarrier.newLayout = newLayout;

        vk::ImageAspectFlags aspectMask = (newLayout == vk::ImageLayout::eDepthAttachmentOptimal) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
        imageBarrier.subresourceRange = imageSubresourceRange(aspectMask);
        imageBarrier.image = image;

        vk::DependencyInfo depInfo{};
        depInfo.pNext = nullptr;

        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &imageBarrier;

        cmd.pipelineBarrier2(&depInfo);
    }

    vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlags aspectMask) {
        vk::ImageSubresourceRange subresRange{};
        subresRange.aspectMask = aspectMask;
        subresRange.baseMipLevel = 0;
        subresRange.levelCount = vk::RemainingMipLevels;
        subresRange.baseArrayLayer = 0;
        subresRange.layerCount = vk::RemainingArrayLayers;
        return subresRange;
    }



    vk::SemaphoreSubmitInfo semaphoreSubmitInfo(
        vk::PipelineStageFlags2 stageMask,
        vk::Semaphore semaphore
    ) {
        vk::SemaphoreSubmitInfo submitInfo{};
    	submitInfo.pNext = nullptr;
    	submitInfo.semaphore = semaphore;
    	submitInfo.stageMask = stageMask;
    	submitInfo.deviceIndex = 0;
    	submitInfo.value = 1;

    	return submitInfo;
    }


    vk::CommandBufferSubmitInfo commandBufferSubmitInfo(
        vk::CommandBuffer cmd
    ) {
        vk::CommandBufferSubmitInfo info{};
    	info.pNext = nullptr;
    	info.commandBuffer = cmd;
    	info.deviceMask = 0;

    	return info;
    }

    vk::SubmitInfo2 submitInfo(
        vk::CommandBufferSubmitInfo* cmd,
        vk::SemaphoreSubmitInfo* signalSemaphoreInfo,
        vk::SemaphoreSubmitInfo* waitSemaphoreInfo
    ) {
        vk::SubmitInfo2 info = {};
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
