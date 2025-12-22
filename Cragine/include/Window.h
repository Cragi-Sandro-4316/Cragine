#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>


namespace crg {




    class Window {
    public:
        Window(
            int width,
            int height,
            std::string name
        );
        // ~Window();

        Window (const Window&) = delete;
        Window& operator= (const Window&) = delete;

        GLFWwindow* getGlfwWindow();

        uint32_t getWidth() {
            return m_width;
        }

        uint32_t getHeight() {
            return m_height;
        }

    private:
        void initWindow();


    private:
        GLFWwindow* m_window;


        int m_width;
        int m_height;

        std::string m_windowName;

    };




}
