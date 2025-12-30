#include "Swapchain.h"
#include "Renderer/Device/Device.h"
#include "VkBootstrap.h"
#include "utils/Logger.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace crg::renderer {

    Swapchain::Swapchain(Window* window, Device* device, Instance* instance) :
    m_window(window),
    m_device(device),
    m_instance(instance) {
        auto width = m_window->getWidth();
        auto height = m_window->getHeight();


        createSwapchain(width, height);
    }

    void Swapchain::createSwapchain(
        uint32_t width,
        uint32_t height
    ) {
        LOG_CORE_INFO("Creating swapchain");

        vkb::SwapchainBuilder swapchainBuilder { m_device->getPhysicalDevice(), m_device->getDevice(), m_instance->getSurface() };

        m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

        vkb::Swapchain vkbSwapchain = swapchainBuilder
            .set_desired_format(VkSurfaceFormatKHR {
                .format = m_swapchainImageFormat,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            })
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
        .value();

        m_swapchainExtent = vkbSwapchain.extent;
        m_swapchain = vkbSwapchain.swapchain;
        m_swapchainImages = vkbSwapchain.get_images().value();
        m_swapchainImageViews = vkbSwapchain.get_image_views().value();


    }

}
