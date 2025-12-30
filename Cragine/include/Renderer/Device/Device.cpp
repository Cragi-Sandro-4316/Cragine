#include "Device.h"
#include "Renderer/macros.h"
#include "VkBootstrap.h"
#include "utils/Logger.h"
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace crg::renderer {

    Device::Device(Instance* instance) :
    m_instance(instance) {
        selectDevice();

        m_graphicsQueue = m_vkbDevice.get_queue(vkb::QueueType::graphics).value();
        m_graphycsQueueFamily = m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        initVma();
    }

    void Device::selectDevice() {
        LOG_CORE_INFO("Selecting physical device");

        VkPhysicalDeviceVulkan13Features features { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        features.dynamicRendering = true;
        features.synchronization2 = true;

        VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        vkb::PhysicalDeviceSelector selector {m_instance->getVkbInstance()};
        m_vkbPhysicalDevice = selector
            .set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .set_surface(m_instance->getSurface())
            .select()
        .value();

        vkb::DeviceBuilder deviceBuilder {m_vkbPhysicalDevice};

        m_vkbDevice = deviceBuilder.build().value();
        m_device = m_vkbDevice.device;
        m_physicalDevice = m_vkbPhysicalDevice.physical_device;
    }


    void Device::initVma() {
        VmaAllocatorCreateInfo allocInfo{};
        allocInfo.physicalDevice = m_physicalDevice;
        allocInfo.device = m_device;
        allocInfo.instance = m_instance->getInstance();
        allocInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        VK_CHECK(vmaCreateAllocator(&allocInfo, &m_allocator));

        m_deletionQueue.pushFunction([&]() {
            vmaDestroyAllocator(m_allocator);
        });
    }


}
