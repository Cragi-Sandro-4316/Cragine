#include "Renderer.h"
#include "VkBootstrap.h"
#include "utils/Logger.h"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>


namespace crg::renderer {

    Renderer::Renderer(GLFWwindow* window) :
    m_window(window) {
        LOG_CORE_INFO("Initialising renderer");

        makeInstance("Cragine");
        selectDevice();
    }

    Renderer::~Renderer() {
        LOG_CORE_INFO("Destroying renderer");
        while (m_deletionQueue.size() > 0) {
            m_deletionQueue.back()(m_instance);
            m_deletionQueue.pop_back();
        }
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
        auto err = glfwCreateWindowSurface(m_instance, m_window, nullptr, (VkSurfaceKHR*)(&m_surface));

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

}
