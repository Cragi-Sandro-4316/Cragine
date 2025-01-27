#include "window/platforms/LinuxWindow.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "utils/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Cragine {

    static bool GLFWInitialized = false;


    static void glfwErrorCallback(int error, const char* description) {
        LOG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Window* Window::create(const WindowProps& props) {
        return new LinuxWindow(props);
    }

    LinuxWindow::LinuxWindow(const WindowProps& props) {
        init(props);
    }

    LinuxWindow::~LinuxWindow() {}

    void LinuxWindow::init(const WindowProps& props) {
        data.title = props.title;
        data.width = props.width;
        data.height = props.height;

        LOG_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

        if (!GLFWInitialized) {
            int success = glfwInit();

            if (!success) {
                LOG_FATAL("Could not initialize GLFW!");
                return;
            }
            glfwSetErrorCallback(glfwErrorCallback);
            GLFWInitialized = true;
        }

        // create the window
        window = glfwCreateWindow((int)props.width, (int)props.height, data.title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window);

        // initialize glad
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!status) {
            LOG_CORE_FATAL("Failed to initialize glad");
        }

        glfwSetWindowUserPointer(window, &data);
        setVSync(true);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height){
            WindowData* data = (WindowData*) glfwGetWindowUserPointer(window);

            // update the size variables
            data->width = width;
            data->height = height;

            // generate the event
            WindowResizeEvent event(width, height);
            data->eventCallback(event);
            
        });


        glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
            WindowData* data = (WindowData*) glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data->eventCallback(event);
        });

        glfwSetKeyCallback(window, [](
            GLFWwindow* window,
            int key,
            int scancode, 
            int action, 
            int mods
        ) {
            WindowData* data = (WindowData*) glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    data->eventCallback(event);
                    break;
                }
                
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data->eventCallback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, 1);
                    data->eventCallback(event);
                    break;
                }
            }

        });


        glfwSetMouseButtonCallback(window, [](
            GLFWwindow* window,
            int key,
            int action, 
            int mods
        ) {
            WindowData* data = (WindowData*) glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(key);
                    data->eventCallback(event);
                    break;
                }
                
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(key);
                    data->eventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(window, [](
            GLFWwindow* window,
            double xOffset, 
            double yOffset
        ) {
            WindowData* data = (WindowData*) glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data->eventCallback(event);
        });

        glfwSetCursorPosCallback(window, [](
            GLFWwindow* window, 
            double xpos, 
            double ypos
        ){
            WindowData* data = (WindowData*) glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xpos, (float)ypos);
            data->eventCallback(event);
        });




        glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int keycode) {
            
            WindowData* data = (WindowData*) glfwGetWindowUserPointer(window);
            KeyTypedEvent event(keycode);

            data->eventCallback(event);

        });


    }


    void LinuxWindow::shutdown() {
        glfwDestroyWindow(window);
    }

    void LinuxWindow::onUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    void LinuxWindow::setVSync(bool enabled) {
        if (enabled)
            glfwSwapInterval(1);
        else 
            glfwSwapInterval(0);

        data.VSync = enabled;
    }

    bool LinuxWindow::isVSync() const {
        return data.VSync;
    }

}