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

    vk::FenceCreateInfo fenceCreateInfo(
        vk::FenceCreateFlags flags
    );

    vk::SemaphoreCreateInfo semaphoreCreateInfo(
        vk::SemaphoreCreateFlags flags
    );

    vk::CommandBufferBeginInfo commandBufferBeginInfo(
        vk::CommandBufferUsageFlags flags
    );

    void transitionImage(
        vk::CommandBuffer cmd,
        vk::Image image,
        vk::ImageLayout currentlayout,
        vk::ImageLayout newLayout
    );

    vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlags aspectMask);

    vk::SemaphoreSubmitInfo semaphoreSubmitInfo(
        vk::PipelineStageFlags2 stageMask,
        vk::Semaphore semaphore
    );


    vk::CommandBufferSubmitInfo commandBufferSubmitInfo(
        vk::CommandBuffer cmd
    );

    vk::SubmitInfo2 submitInfo(
        vk::CommandBufferSubmitInfo* cmd,
        vk::SemaphoreSubmitInfo* signalSemaphoreInfo,
        vk::SemaphoreSubmitInfo* waitSemaphoreInfo
    );

    vk::ImageViewCreateInfo imageViewCreateInfo(
        vk::Format format,
        vk::Image image,
        vk::ImageAspectFlags aspectFlags
    );

    vk::ImageCreateInfo imageCreateInfo(
        vk::Format format,
        vk::ImageUsageFlags flags,
        vk::Extent3D extent
    );

    void copyImageToImage(
        vk::CommandBuffer cmd,
        vk::Image source,
        vk::Image destination,
        vk::Extent2D srcSize,
        vk::Extent2D dstSize
    );

}
