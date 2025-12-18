#pragma once

#include <deque>
#include <functional>
#define VUILKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace crg {

    class Renderer {
    public:
        Renderer(GLFWwindow* window);

        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer operator=(Renderer&) = delete;

    private:

        // @brief Destruction queue
        std::deque<std::function<void()>> m_deletionQueue;

        vk::Instance m_instance = VK_NULL_HANDLE;

        GLFWwindow* m_window;

    };

}
