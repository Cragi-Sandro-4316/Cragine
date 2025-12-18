#include "Instance.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace crg {
    vk::Instance makeInstance(
        const char *appName,
        std::deque<std::function<void(vk::Instance)>> &deletionQueue
    ) {
        LOG_CORE_TRACE("Making an instance...");

        vkb::InstanceBuilder builder{};

        builder
            .set_app_name(appName)
            .set_engine_name("Cragine")
            .require_api_version(1, 3, 0)
            .request_validation_layers()
            .use_default_debug_messenger();

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


        LOG_CORE_INFO("Supported extensions: ");
        auto supportedExts = vk::enumerateInstanceExtensionProperties();
        for (vk::ExtensionProperties& ext : supportedExts) {
            LOG_CORE_TRACE("{}", (char*)ext.extensionName);
        }

        LOG_CORE_INFO("Requested extensions: ");
        for (int i = 0; i < glfwExtensionCount; i++) {
            LOG_CORE_TRACE("{}", glfwExtensions[i]);
        }

        builder.enable_extensions(glfwExtensionCount, glfwExtensions);

        auto result = builder.build();

        if (!result) {
            LOG_CORE_ERROR("Failed to create VkInstance: ");
            LOG_CORE_ERROR("{}", result.error().message());
            return nullptr;
        }

        vkb::Instance vkbInstance = result.value();
        vk::Instance instance = vkbInstance.instance;


        deletionQueue.emplace_back([vkbInstance](vk::Instance) {
            vkb::destroy_instance(vkbInstance);
            LOG_CORE_TRACE("Deleted instance");
        });

        return instance;
    }
}
