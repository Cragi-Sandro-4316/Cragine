#include "Instance.h"
#include "VkBootstrap.h"
#include "utils/Logger.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace crg::renderer {

    Instance::Instance(const char* appName, Window* window)
    : m_window(window) {
        createInstance(appName);

        createSurface();
    }

    void Instance::createInstance(const char* appName) {
        constexpr bool validationActive = true;
        LOG_CORE_INFO("Creating vkInstance");

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vkb::InstanceBuilder builder;

        if (validationActive) {
            builder
                .request_validation_layers(validationActive)
                .use_default_debug_messenger();
        }

        auto ret = builder
            .set_app_name(appName)
            .enable_extensions(glfwExtensionCount, glfwExtensions)
            .require_api_version(1, 3, 0)
        .build();

        m_vkbInstance = ret.value();
        m_instance = m_vkbInstance.instance;
        m_debugMessanger = m_vkbInstance.debug_messenger;
    }

    void Instance::createSurface() {
        auto err = glfwCreateWindowSurface(m_instance, m_window->getGlfwWindow(), nullptr, &m_surface);

    }

}
