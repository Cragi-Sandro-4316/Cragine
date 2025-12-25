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

    vk::ImageCreateInfo imageCreateInfo(
        vk::Format format,
        vk::ImageUsageFlags flags,
        vk::Extent3D extent
    ) {
        vk::ImageCreateInfo info{};
        info.pNext = nullptr;

        info.imageType = vk::ImageType::e2D;

        info.format = format;
        info.extent = extent;

		info.mipLevels = 1;
    	info.arrayLayers = 1;

        info.samples = vk::SampleCountFlagBits::e1;

        info.tiling = vk::ImageTiling::eOptimal;
        info.usage = flags;

        return info;
    }

    vk::ImageViewCreateInfo imageViewCreateInfo(
        vk::Format format,
        vk::Image image,
        vk::ImageAspectFlags aspectFlags
    ) {
        vk::ImageViewCreateInfo info{};
        info.pNext = nullptr;

        info.viewType = vk::ImageViewType::e2D;
        info.image = image;
        info.format = format;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.subresourceRange.aspectMask = aspectFlags;

        return info;
    }

    void copyImageToImage(
        vk::CommandBuffer cmd,
        vk::Image source,
        vk::Image destination,
        vk::Extent2D srcSize,
        vk::Extent2D dstSize
    ) {
        vk::ImageBlit2 blitRegion{};
        blitRegion.pNext = nullptr;

    	blitRegion.srcOffsets[1].x = srcSize.width;
    	blitRegion.srcOffsets[1].y = srcSize.height;
    	blitRegion.srcOffsets[1].z = 1;

    	blitRegion.dstOffsets[1].x = dstSize.width;
    	blitRegion.dstOffsets[1].y = dstSize.height;
    	blitRegion.dstOffsets[1].z = 1;

    	blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    	blitRegion.srcSubresource.baseArrayLayer = 0;
    	blitRegion.srcSubresource.layerCount = 1;
    	blitRegion.srcSubresource.mipLevel = 0;

    	blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    	blitRegion.dstSubresource.baseArrayLayer = 0;
    	blitRegion.dstSubresource.layerCount = 1;
    	blitRegion.dstSubresource.mipLevel = 0;

    	vk::BlitImageInfo2 blitInfo{};
        blitInfo.pNext = nullptr;

    	blitInfo.dstImage = destination;
    	blitInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
    	blitInfo.srcImage = source;
    	blitInfo.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
    	blitInfo.filter = vk::Filter::eLinear;
    	blitInfo.regionCount = 1;
    	blitInfo.pRegions = &blitRegion;

    	cmd.blitImage2(&blitInfo);
    }

}
