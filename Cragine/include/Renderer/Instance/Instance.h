#pragma once

#include "VkBootstrap.h"
#include "Window.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace crg::renderer {
    class Instance {
    public:
        Instance(const char* appName, Window* window);

        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;

        VkSurfaceKHR& getSurface() { return m_surface; }
        VkInstance& getInstance() { return m_instance; }
        vkb::Instance& getVkbInstance() { return m_vkbInstance; }
    private:
        void createInstance(const char* appName);

        void createSurface();
    private:
        VkInstance m_instance                           = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessanger       = VK_NULL_HANDLE;

        VkSurfaceKHR m_surface                          = VK_NULL_HANDLE;

        vkb::Instance m_vkbInstance;



        Window* m_window = nullptr;
    };
}
