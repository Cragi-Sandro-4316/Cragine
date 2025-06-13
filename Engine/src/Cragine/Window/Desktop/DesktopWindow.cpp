#include "../../CraginePCH.h"
#include "DesktopWindow.h"

namespace Cragine {
    static bool s_GLFWInitialized = false;


    Window* Window::Create(const WindowProps& props) {
        return new DesktopWindow(props);
    }

    DesktopWindow::DesktopWindow(const WindowProps& props) {
        init(props);
    }

    DesktopWindow::~DesktopWindow() {
        shutdown();
    }

    void DesktopWindow::init(const WindowProps& props) {
        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;

        LOG_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

        if (!s_GLFWInitialized) {
            int success = glfwInit();

            
            CRAGINE_ASSERT(success, "Could not initialize GLFW!")
            

            s_GLFWInitialized = true;
        }

        m_window = glfwCreateWindow(
            (int) props.width,
            (int) props.height,
            props.title.c_str(),
            nullptr,
            nullptr
        );

        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, &m_data);
        setVSync(true);
    }

    void DesktopWindow::shutdown() {
        glfwDestroyWindow(m_window);
    }

    void DesktopWindow::onUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }

    void DesktopWindow::setVSync(bool enabled) {
        if (enabled) 
            glfwSwapInterval(1);
        
    }

    bool DesktopWindow::isVSync() const {
        return true;
    }
}