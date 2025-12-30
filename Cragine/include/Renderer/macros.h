#pragma once

#include <vulkan/vk_enum_string_helper.h>
#include "utils/Logger.h"

#define VK_CHECK(x)                                                             \
    do {                                                                        \
    VkResult err = x;                                                           \
        if (err) {                                                              \
            LOG_CORE_ERROR("Detected vulkan error: {}", string_VkResult(err));  \
            abort();                                                            \
        }                                                                       \
    } while(0)
