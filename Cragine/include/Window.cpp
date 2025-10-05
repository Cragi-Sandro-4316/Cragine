#include "Window.h"
#include <iostream>

namespace crg {
    Window::Window(
        int width,
        int height,
        std::string name
    ) :
    m_width(width),
    m_height(height),
    m_windowName(name) {
        initWindow();

    }

    void Window::initWindow() {
        std::cout<<"window created"<<std::endl;
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window = glfwCreateWindow(
            m_width,
            m_height,
            m_windowName.c_str(),
            nullptr,
            nullptr
        );

        glfwSetWindowUserPointer(m_window, this);
    }

    GLFWwindow* Window::getGlfwWindow() {
        return m_window;
    }

}
