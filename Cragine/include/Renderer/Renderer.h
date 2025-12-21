#pragma once

#include "VkBootstrap.h"
#include "vulkan/vulkan.hpp"
#include <deque>
#include <functional>
#include <vulkan/vulkan_core.h>
#define VUILKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace crg::renderer {

    class Renderer {
    public:
        Renderer(GLFWwindow* window);

        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer operator=(Renderer&) = delete;



    private:
        void makeInstance(const char* appName);

        void selectDevice();

    private:

        // @brief Destruction queue
        std::deque<std::function<void(vk::Instance)>> m_deletionQueue;

        vk::Instance m_instance = VK_NULL_HANDLE;

        vk::DebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

        vk::SurfaceKHR m_surface = VK_NULL_HANDLE;

        vk::PhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

        vk::Device m_device = VK_NULL_HANDLE;



        GLFWwindow* m_window;


        vkb::Instance m_vkbInstance;
        vkb::PhysicalDevice m_vkbPhysicalDevice;
        vkb::Device m_vkbDevice;
    };

}
