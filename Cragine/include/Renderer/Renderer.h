#pragma once

#include "Renderer/Descriptors.h"
#include "utils/VkTypes.h"
#include "VkBootstrap.h"
#include "Window.h"
#include "utils/Logger.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <deque>
#include <functional>
#include <vulkan/vulkan_core.h>
#define VUILKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace crg::renderer {

    #define VK_CHECK(x)                                                         \
    do {                                                                        \
    vk::Result err = x;                                                         \
        if ((int)err) {                                                         \
            LOG_CORE_ERROR("Detected vulkan error: {}", vk::to_string(err));    \
            abort();                                                            \
        }                                                                       \
    } while(0)                                                                  \


    class Renderer {
    public:
        Renderer(Window* window);

        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer operator=(Renderer&) = delete;

        FrameData& getCurrentFrame() { return m_frames[m_frameNumber % m_frames.size()]; }

        void draw();

        void drawBackground(vk::CommandBuffer cmd);

    public:
        std::vector<FrameData> m_frames;

        vk::Queue m_graphicsQueue;
        uint32_t m_graphicsQueueFamily;

        DescriptorAllocator m_globalDescriptorAllocator;

        vk::DescriptorSet m_drawImageDescriptors;
        vk::DescriptorSetLayout m_drawImageDescriptorLayout;

        vk::Pipeline m_gradientPipeline;
        vk::PipelineLayout m_gradientPipelineLayout;

    private:
        void makeInstance(const char* appName);

        void selectDevice();

        void createSwapchain(uint32_t width, uint32_t height);

        void initSwapchain();

        void destroySwapchain();

        void cleanup();

        void initCommands();

        void initSyncStructs();

        void initDescriptors();

        void initPipelines();
        void initBackgroundPipelines();

    private:

        uint32_t m_frameNumber = 0;

        VmaAllocator m_allocator;

        DeletionQueue m_deletionQueue;

        AllocatedImage m_drawImage;

        vk::Extent2D m_drawExtent;

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

        vkb::Instance m_vkbInstance;
        vkb::PhysicalDevice m_vkbPhysicalDevice;
        vkb::Device m_vkbDevice;
    };

}
