#pragma once

#include "Cragine/CraginePCH.h"
#include "Cragine/Core.h"
#include "Cragine/Events/Event.h"

namespace Cragine {

    struct WindowProps {
        std::string title;
        unsigned int width;
        unsigned int height;

        WindowProps(
            const std::string& title = "Cragine engine", 
            unsigned int width = 1280,
            unsigned int height = 720
        ) : title(title), width(width), height(height) {}
    };

    // Common Window interface
    class Window {
    public: 
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() {}

        virtual void onUpdate() = 0;

        virtual unsigned int getWidth() const = 0;
        virtual unsigned int getHeight() const = 0;

        // Attributes
        virtual void setEventCallback(const EventCallbackFn& callback) = 0;
        virtual void setVSync(bool enabled) = 0;
        virtual bool isVSync() const = 0;

        static Window* Create(const WindowProps& props = WindowProps());
    };
}