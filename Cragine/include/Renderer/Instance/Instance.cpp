#include "Instance.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

namespace crg {
    vk::Instance makeInstance(
        const char *appName,
        std::deque<std::function<void ()>> &deletionQueue
    ) {
        LOG_CORE_TRACE("Making an instance...");

        vkb::InstanceBuilder builder;

        auto result = builder
            .set_app_name(appName)
            .set_engine_name("Cragine")
            .require_api_version(1, 3, 0)
            .use_default_debug_messenger()
        .build();

        if (!result) {
            LOG_CORE_ERROR("Failed to create VkInstance");
            return nullptr;
        }

        vkb::Instance vkbInstance = result.value();
        vk::Instance instance = vkbInstance.instance;

        deletionQueue.emplace_back([vkbInstance]() {
            vkb::destroy_instance(vkbInstance);
            LOG_CORE_TRACE("Deleted instance");
        });

        return instance;
    }
}
