#pragma once

#define VUILKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <deque>
#include <functional>


namespace crg {

    /**
     * @brief Creates a Vulkan instance
     *
     * @param appName the application name
     * @param deletionQueue queue onto which to push the instance's destructor
     *
     * @return the instance created.
     */
    vk::Instance makeInstance(const char* appName, std::deque<std::function<void(vk::Instance)>>& deletionQueue);
}
