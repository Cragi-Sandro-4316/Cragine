#pragma once

#include "../Instance/Instance.h"
#include "../utils/utils.h"
#include "VkBootstrap.h"
#include "Window.h"

#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace crg::renderer {
    class Device {
    public:
        Device(Instance* instance);

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        VkPhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
        VkDevice& getDevice() { return m_device; }

        VmaAllocator& getAllocator() { return m_allocator; }

        uint32_t getGraphicsFamilyIndex() { return m_graphycsQueueFamily; }

        VkQueue getGraphicsQueue() { return m_graphicsQueue; }

    private:
        void initVulkan();

        void selectDevice();

        void initVma();

    private:

        utils::DeletionQueue m_deletionQueue;

        Window* m_window = nullptr;

        VkPhysicalDevice m_physicalDevice   = VK_NULL_HANDLE;
        VkDevice m_device                   = VK_NULL_HANDLE;

        VkQueue m_graphicsQueue;
        uint32_t m_graphycsQueueFamily;

        VmaAllocator m_allocator;

        Instance* m_instance;


        vkb::PhysicalDevice m_vkbPhysicalDevice;
        vkb::Device m_vkbDevice;
    };
}
