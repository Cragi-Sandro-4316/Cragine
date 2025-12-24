#include "Renderer.h"
#include "Renderer/utils/RendererUtils.h"
#include "VkBootstrap.h"
#include "utils/Logger.h"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>


namespace crg::renderer {

    Renderer::Renderer(Window* window) :
    m_window(window) {
        LOG_CORE_INFO("Initialising renderer");

        makeInstance("Cragine");
        selectDevice();
        initSwapchain();

        m_graphicsQueue = m_vkbDevice.get_queue(vkb::QueueType::graphics).value();
        m_graphicsQueueFamily = m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        initCommands();
        initSyncStructs();

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
        m_frames.resize(m_swapchainImages.size());
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

            for (uint32_t i = 0; i < m_frames.size(); i++) {
                m_device.destroyCommandPool(m_frames[i].m_commandPool);

                m_device.destroyFence(m_frames[i].m_renderFence, nullptr);
                m_device.destroySemaphore(m_frames[i].m_renderSemaphore, nullptr);
                m_device.destroySemaphore(m_frames[i].m_swapchainSemaphore, nullptr);
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

        for (uint32_t i = 0; i < m_frames.size(); i++) {
            VK_CHECK(m_device.createCommandPool(&commandPoolInfo, nullptr, &m_frames[i].m_commandPool));

            vk::CommandBufferAllocateInfo cmdAllocInfo = utils::commandBufferAllocateInfo(
                m_frames[i].m_commandPool,
                1
            );

            VK_CHECK(m_device.allocateCommandBuffers(&cmdAllocInfo, &m_frames[i].m_mainComandBuffer));
        }

    }

    void Renderer::initSyncStructs() {
       	//create syncronization structures
    	//one fence to control when the gpu has finished rendering the frame,
    	//and 2 semaphores to syncronize rendering with swapchain
    	//we want the fence to start signalled so we can wait on it on the first frame
        vk::FenceCreateInfo fenceCreateInfo = utils::fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
        vk::SemaphoreCreateInfo semaphoreCreateInfo = utils::semaphoreCreateInfo(vk::SemaphoreCreateFlags{});

        for (uint32_t i = 0; i < m_frames.size(); i++) {
            VK_CHECK(m_device.createFence(&fenceCreateInfo, nullptr, &m_frames[i].m_renderFence));

            VK_CHECK(m_device.createSemaphore(&semaphoreCreateInfo, nullptr, &m_frames[i].m_swapchainSemaphore));
            VK_CHECK(m_device.createSemaphore(&semaphoreCreateInfo, nullptr, &m_frames[i].m_renderSemaphore));
        }
    }

    void Renderer::draw() {
        // Wait until the gpu is finished. Timeout of 1 second.
        VK_CHECK(m_device.waitForFences(1, &getCurrentFrame().m_renderFence, true, 1000000000));
        VK_CHECK(m_device.resetFences(1, &getCurrentFrame().m_renderFence));

        uint32_t swapchainImageIndex;
        VK_CHECK(m_device.acquireNextImageKHR(m_swapchain, 1000000000, getCurrentFrame().m_swapchainSemaphore, nullptr, &swapchainImageIndex));


        vk::CommandBuffer cmd = getCurrentFrame().m_mainComandBuffer;

        cmd.reset(vk::CommandBufferResetFlagBits{});

        vk::CommandBufferBeginInfo cmdBeginInfo = utils::commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        VK_CHECK(cmd.begin(&cmdBeginInfo));

        utils::transitionImage(
            cmd,
            m_swapchainImages[swapchainImageIndex],
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eGeneral
        );

        //make a clear-color from frame number. This will flash with a 120 frame period.
        vk::ClearColorValue clearValue{};
        float flash = std::abs(std::sin(m_frameNumber / 120.f));
        clearValue = vk::ClearColorValue{ 0.0f, 0.0f, flash, 1.0f };

        vk::ImageSubresourceRange clearRange = utils::imageSubresourceRange(vk::ImageAspectFlagBits::eColor);

        //clear image
        cmd.clearColorImage(
            m_swapchainImages[swapchainImageIndex],
            vk::ImageLayout::eGeneral,
            &clearValue,
            1,
            &clearRange
        );

        utils::transitionImage(
            cmd,
            m_swapchainImages[swapchainImageIndex],
            vk::ImageLayout::eGeneral,
            vk::ImageLayout::ePresentSrcKHR
        );

        cmd.end();

        vk::CommandBufferSubmitInfo cmdInfo = utils::commandBufferSubmitInfo(cmd);

        vk::SemaphoreSubmitInfo waitInfo = utils::semaphoreSubmitInfo(
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            getCurrentFrame().m_swapchainSemaphore
        );
        vk::SemaphoreSubmitInfo signalInfo = utils::semaphoreSubmitInfo(
            vk::PipelineStageFlagBits2::eAllGraphics,
            getCurrentFrame().m_renderSemaphore
        );

        vk::SubmitInfo2 submit = utils::submitInfo(
            &cmdInfo,
            &signalInfo,
            &waitInfo
        );

        VK_CHECK(m_graphicsQueue.submit2(1, &submit, getCurrentFrame().m_renderFence));


        vk::PresentInfoKHR presentInfo{};
        presentInfo.pNext = nullptr;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.swapchainCount = 1;

        presentInfo.pWaitSemaphores = &getCurrentFrame().m_renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &swapchainImageIndex;

        VK_CHECK(m_graphicsQueue.presentKHR(&presentInfo));

        m_frameNumber++;
    }



}
