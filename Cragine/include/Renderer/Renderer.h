#pragma once

#include "VkBootstrap.h"
#include "Window.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <deque>
#include <functional>
#include <vulkan/vulkan_core.h>
#define VUILKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace crg::renderer {

    struct FrameData {
        vk::CommandPool m_commandPool;
        vk::CommandBuffer m_mainComandBuffer;
    };

    constexpr uint32_t FRAME_OVERLAP = 2;

    class Renderer {
    public:
        Renderer(Window* window);

        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer operator=(Renderer&) = delete;

        FrameData& getCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; }

    public:
        FrameData m_frames[FRAME_OVERLAP];

        vk::Queue m_graphicsQueue;
        uint32_t m_graphicsQueueFamily;

    private:
        void makeInstance(const char* appName);

        void selectDevice();

        void createSwapchain(uint32_t width, uint32_t height);

        void initSwapchain();

        void destroySwapchain();

        void cleanup();

        void initCommands();

    private:

        // std::deque<std::function<void(vk::Instance)>> m_deletionQueue;

        uint32_t m_frameNumber = 0;

        vk::Instance m_instance = VK_NULL_HANDLE;

        vk::DebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

        vk::SurfaceKHR m_surface = VK_NULL_HANDLE;

        vk::PhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

        vk::Device m_device = VK_NULL_HANDLE;

        vk::SwapchainKHR m_swapchain = VK_NULL_HANDLE;

        vk::Format m_swapchainImageFormat;

        std::vector<VkImage> m_swapchainImages;
        std::vector<VkImageView> m_swapchainImageViews;
        vk::Extent2D m_swapchainExtent;

        Window* m_window;

        bool m_isInitialized = false;

        // TODO: Push deletion lambdas to deletion queue


        vkb::Instance m_vkbInstance;
        vkb::PhysicalDevice m_vkbPhysicalDevice;
        vkb::Device m_vkbDevice;
    };

}
