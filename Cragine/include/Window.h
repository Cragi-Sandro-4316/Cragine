#pragma once
#include <GLFW/glfw3.h>
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

    private:
        void initWindow();


    private:
        GLFWwindow* m_window;


        int m_width;
        int m_height;

        std::string m_windowName;

    };
}
