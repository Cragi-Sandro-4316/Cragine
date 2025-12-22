#include "Renderer.h"
#include "Renderer/utils/RendererUtils.h"
#include "VkBootstrap.h"
#include "utils/Logger.h"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>


namespace crg::renderer {

    Renderer::Renderer(Window* window) :
    m_window(window) {
        LOG_CORE_INFO("Initialising renderer");

        makeInstance("Cragine");
        selectDevice();
        initSwapchain();

        m_graphicsQueue = m_vkbDevice.get_queue(vkb::QueueType::graphics).value();
        m_graphicsQueueFamily = m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        m_isInitialized = true;
    }

    Renderer::~Renderer() {
        LOG_CORE_INFO("Destroying renderer");
        // while (m_deletionQueue.size() > 0) {
        //     m_deletionQueue.back()(m_instance);
        //     m_deletionQueue.pop_back();
        // }
        //
        cleanup();
    }

    void Renderer::makeInstance(const char* appName) {
        LOG_CORE_INFO("Creating vkInstance");

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vkb::InstanceBuilder builder;

        auto ret = builder
            .set_app_name(appName)
            .request_validation_layers(true)
            .enable_extensions(glfwExtensionCount, glfwExtensions)
            .use_default_debug_messenger()
            .require_api_version(1, 3, 0)
        .build();

        m_vkbInstance = ret.value();

        m_instance = m_vkbInstance.instance;
        m_debugMessenger = m_vkbInstance.debug_messenger;

    }

    void Renderer::selectDevice() {
        LOG_CORE_INFO("Selecting physical device");
        auto err = glfwCreateWindowSurface(m_instance, m_window->getGlfwWindow(), nullptr, (VkSurfaceKHR*)(&m_surface));

        VkPhysicalDeviceVulkan13Features features { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        features.dynamicRendering = true;
        features.synchronization2 = true;

        VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        vkb::PhysicalDeviceSelector selector {m_vkbInstance};
        m_vkbPhysicalDevice = selector
            .set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .set_surface(m_surface)
            .select()
        .value();

        vkb::DeviceBuilder deviceBuilder {m_vkbPhysicalDevice};

        m_vkbDevice = deviceBuilder.build().value();
        m_device = m_vkbDevice.device;
        m_physicalDevice = m_vkbPhysicalDevice.physical_device;
    }

    void Renderer::createSwapchain(uint32_t width, uint32_t height) {
        LOG_CORE_INFO("Creating Swapchain...");
        vkb::SwapchainBuilder swapchainBuilder { m_physicalDevice, m_device, m_surface };

        m_swapchainImageFormat = vk::Format::eB8G8R8A8Unorm;

        vkb::Swapchain vkbSwapchain = swapchainBuilder
            .set_desired_format(VkSurfaceFormatKHR {
                .format = (VkFormat)m_swapchainImageFormat,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            })
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
        .value();

        m_swapchainExtent = vkbSwapchain.extent;
        m_swapchain = vkbSwapchain.swapchain;
        auto images = vkbSwapchain.get_images().value();
        auto views = vkbSwapchain.get_image_views().value();

        m_swapchainImages.resize(images.size());
        m_swapchainImageViews.resize(views.size());

        std::memcpy(m_swapchainImages.data(), images.data(), images.size() * sizeof(VkImage));
        std::memcpy(m_swapchainImageViews.data(), views.data(), views.size() * sizeof(VkImageView));

    }

    void Renderer::initSwapchain() {
        auto width = m_window->getWidth();
        auto height = m_window->getHeight();

        createSwapchain(width, height);
    }

    void Renderer::destroySwapchain() {
        m_device.destroySwapchainKHR(m_swapchain);

        for (auto& imageView : m_swapchainImageViews) {
            m_device.destroyImageView(imageView);
        }
    }

    void Renderer::cleanup() {
        if (m_isInitialized) {
            m_device.waitIdle();

            for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
                m_device.destroyCommandPool(m_frames[i].m_commandPool);
            }

            destroySwapchain();

            m_instance.destroySurfaceKHR(m_surface);
            m_device.destroy();

            vkb::destroy_debug_utils_messenger(m_instance, m_debugMessenger);
            m_instance.destroy();
        }
    }

    void Renderer::initCommands() {
        vk::CommandPoolCreateInfo commandPoolInfo = utils::commandPoolCreateInfo(
            m_graphicsQueueFamily,
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer
        );

        for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
            vk::Result createResult = m_device.createCommandPool(&commandPoolInfo, nullptr, &m_frames[i].m_commandPool);

            if (createResult != vk::Result::eSuccess) {
                LOG_CORE_ERROR("Could not create command pool");
                return;
            }

            vk::CommandBufferAllocateInfo cmdAllocInfo = utils::commandBufferAllocateInfo(
                m_frames[i].m_commandPool,
                1
            );

            vk::Result allocResult = m_device.allocateCommandBuffers(&cmdAllocInfo, &m_frames[i].m_mainComandBuffer);

            if (allocResult != vk::Result::eSuccess) {
                LOG_CORE_ERROR("Could not allocate command buffers");
                return;
            }
        }

    }

}
