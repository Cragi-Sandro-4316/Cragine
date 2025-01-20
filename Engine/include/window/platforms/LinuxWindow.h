#pragma once

#include "window/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Cragine {
    class LinuxWindow : public Window {
    public:
        LinuxWindow(const WindowProps& props);
        virtual ~LinuxWindow();

        void onUpdate() override;

        unsigned int getWidth() const override { return data.width; }
        unsigned int getHeight() const override { return data.height; }

        void setEventCallback(const EventCallbackFn& callback) override { data.eventCallback = callback; }
        void setVSync(bool enabled) override;
        bool isVSync() const override;

    private:
        virtual void init(const WindowProps& props);
        virtual void shutdown();

        GLFWwindow* window;

        struct WindowData {
            std::string title;
            unsigned int width, height;
            bool VSync;

            EventCallbackFn eventCallback;
        };

        WindowData data;
    };
}
