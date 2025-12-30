#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "../utils/utils.h"
#include "../Device/Device.h"
#include "Window.h"

namespace crg::renderer {

    struct AllocatedImage  {
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkExtent3D imageExtent;
        VkFormat imageFormat;
    };

    struct FrameData {
        VkCommandPool m_commandPool;
        VkCommandBuffer m_mainComandBuffer;

        VkSemaphore m_swapchainSemaphore;
        VkSemaphore m_renderSemaphore;
        VkFence m_renderFence;

        utils::DeletionQueue m_deletionQueue;
    };


    class Swapchain {
    public:

        Swapchain(Window* window, Device* device, Instance* instance);

        const std::vector<VkImage>& getImages() { return m_swapchainImages; }
        const VkExtent2D& getExtent() { return m_swapchainExtent; }

        VkSwapchainKHR& getSwapchain() { return m_swapchain; }

    private:

        void createSwapchain(uint32_t width, uint32_t height);

    private:
        Window* m_window;

        Device* m_device;

        Instance* m_instance;

        utils::DeletionQueue m_deletionQueue;

        VkSwapchainKHR m_swapchain      = VK_NULL_HANDLE;

        VkFormat m_swapchainImageFormat;

        std::vector<VkImage> m_swapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;

        VkExtent2D m_swapchainExtent;


    };
}
