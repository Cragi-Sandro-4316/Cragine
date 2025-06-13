#pragma once

#include "../Window.h"

#include <GLFW/glfw3.h>

namespace Cragine {
    class DesktopWindow : public Window {
    public:
        DesktopWindow(const WindowProps& props);
        ~DesktopWindow();

        void onUpdate() override;

        inline unsigned int getWidth() const override { return m_data.width; }
        inline unsigned int getHeight() const override { return m_data.height; }

        // Window attributes
        inline void setEventCallback(const EventCallbackFn& callback) override { m_data.eventCallback = callback; }

        void setVSync(bool enabled) override;
        bool isVSync() const override;
    private: 
        virtual void init(const WindowProps& props);
        virtual void shutdown();

        GLFWwindow* m_window;

        struct WindowData {
            std::string title;
            unsigned int width, height;
            bool VSync;
            EventCallbackFn eventCallback;
        };

        WindowData m_data;
    };
}