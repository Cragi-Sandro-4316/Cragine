#include "Renderer.h"
#include "Instance/Instance.h"
#include "Renderer/Swapchain/Swapchain.h"
#include "Renderer/utils/imageOperations.h"
#include "Renderer/utils/defaults.h"
#include "macros.h"
#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace crg::renderer {

    Renderer::Renderer(Window* window) :
    m_window(window) {
        m_instance = std::make_unique<Instance>("Cragine", m_window);
        m_device = std::make_unique<Device>(m_instance.get());
        m_swapchain = std::make_unique<Swapchain>(m_window, m_device.get(), m_instance.get());

        initFrameData();
        initCommands();
        initSyncStructs();


    }

    void Renderer::initFrameData() {
        m_frames.resize(m_swapchain->getImages().size());
        m_drawExtent = m_swapchain->getExtent();

        VkExtent3D drawImageExtent = {
            m_window->getWidth(),
            m_window->getHeight(),
            1
        };

        m_drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_drawImage.imageExtent = drawImageExtent;

        VkImageUsageFlags drawImageUsages {};
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VkImageCreateInfo rimgInfo = utils::imageCreateInfo(m_drawImage.imageFormat, drawImageUsages, drawImageExtent);

        VmaAllocationCreateInfo rimgAllocInfo{};
        rimgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        rimgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VK_CHECK(vmaCreateImage(
            m_device->getAllocator(),
            &rimgInfo,
            &rimgAllocInfo,
            &m_drawImage.image,
            &m_drawImage.allocation,
            nullptr
        ));


        VkImageViewCreateInfo rviewInfo = utils::imageViewCreateInfo(m_drawImage.imageFormat, m_drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

        VK_CHECK(vkCreateImageView(
            m_device->getDevice(),
            &rviewInfo,
            nullptr,
            &m_drawImage.imageView
        ));

        m_deletionQueue.pushFunction([&]() {
            vkDestroyImageView(m_device->getDevice(), m_drawImage.imageView, nullptr);
            vmaDestroyImage(m_device->getAllocator(), m_drawImage.image, m_drawImage.allocation);
        });
    }

    void Renderer::initCommands() {
        VkCommandPoolCreateInfo commandPoolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_device->getGraphicsFamilyIndex()
        };

        for (int i = 0; i < m_frames.size(); i++) {
            VK_CHECK(vkCreateCommandPool(m_device->getDevice(), &commandPoolInfo, nullptr, &m_frames[i].m_commandPool));

            VkCommandBufferAllocateInfo cmdAllocInfo = utils::commandBufferAllocateInfo(m_frames[i].m_commandPool, 1);

            VK_CHECK(vkAllocateCommandBuffers(
                m_device->getDevice(),
                &cmdAllocInfo,
                &m_frames[i].m_mainComandBuffer
            ));
        }

    }

    void Renderer::initSyncStructs() {
        VkFenceCreateInfo fenceCreateInfo = utils::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
        VkSemaphoreCreateInfo semaphoreCreateInfo = utils::semaphoreCreateInfo(0);

        for (int i = 0; i < m_frames.size(); i++) {
            VK_CHECK(vkCreateFence(
                m_device->getDevice(),
                &fenceCreateInfo,
                nullptr,
                &m_frames[i].m_renderFence
            ));


            VK_CHECK(vkCreateSemaphore(
                m_device->getDevice(),
                &semaphoreCreateInfo,
                nullptr,
                &m_frames[i].m_swapchainSemaphore
            ));

            VK_CHECK(vkCreateSemaphore(
                m_device->getDevice(),
                &semaphoreCreateInfo,
                nullptr,
                &m_frames[i].m_renderSemaphore
            ));
        }
    }

    void Renderer::draw() {
        VK_CHECK(vkWaitForFences(
            m_device->getDevice(),
            1,
            &getCurrentFrame().m_renderFence,
            true,
            1000000000
        ));

        VK_CHECK(vkResetFences(
            m_device->getDevice(),
            1,
            &getCurrentFrame().m_renderFence
        ));

        uint32_t swapchainImageIndex;
        VK_CHECK(vkAcquireNextImageKHR(
            m_device->getDevice(),
            m_swapchain->getSwapchain(),
            1000000000,
            getCurrentFrame().m_swapchainSemaphore,
            nullptr,
            &swapchainImageIndex
        ));


        VkCommandBuffer cmd = getCurrentFrame().m_mainComandBuffer;

        VK_CHECK(vkResetCommandBuffer(cmd, 0));

        VkCommandBufferBeginInfo cmdBeginInfo = utils::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        imageOp::transitionImage(
            cmd,
            m_swapchain->getImages()[swapchainImageIndex],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL
        );

        VkClearColorValue clearValue;
        float flash = std::abs(std::sin(m_frameNumber / 120.f));
        clearValue = { { flash, 0.0f, 0.0f, 1.0f } };

        VkImageSubresourceRange clearRange = imageOp::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdClearColorImage(
            cmd,
            m_swapchain->getImages()[swapchainImageIndex],
            VK_IMAGE_LAYOUT_GENERAL,
            &clearValue,
            1,
            &clearRange
        );

        imageOp::transitionImage(
            cmd,
            m_swapchain->getImages()[swapchainImageIndex],
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        );

        VK_CHECK(vkEndCommandBuffer(cmd));

        VkCommandBufferSubmitInfo cmdInfo = utils::commandBufferSubmitInfo(cmd);

        VkSemaphoreSubmitInfo waitInfo = utils::semaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
            getCurrentFrame().m_swapchainSemaphore
        );

        VkSemaphoreSubmitInfo signalInfo = utils::semaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
            getCurrentFrame().m_renderSemaphore
        );

        VkSubmitInfo2 submit = utils::submitInfo(
            &cmdInfo,
            &signalInfo,
            &waitInfo
        );

        VK_CHECK(vkQueueSubmit2(m_device->getGraphicsQueue(), 1, &submit, getCurrentFrame().m_renderFence));

        VkPresentInfoKHR presentInfo = {};
    	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    	presentInfo.pNext = nullptr;
    	presentInfo.pSwapchains = &m_swapchain->getSwapchain();
    	presentInfo.swapchainCount = 1;

    	presentInfo.pWaitSemaphores = &getCurrentFrame().m_renderSemaphore;
    	presentInfo.waitSemaphoreCount = 1;

    	presentInfo.pImageIndices = &swapchainImageIndex;

    	VK_CHECK(vkQueuePresentKHR(m_device->getGraphicsQueue(), &presentInfo));

    	//increase the number of frames drawn
    	m_frameNumber++;

    }


}
