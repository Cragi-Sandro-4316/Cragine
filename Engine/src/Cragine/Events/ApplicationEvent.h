#pragma once

#include "Event.h"

namespace Cragine {

    class WindowResizeEvent : public Event {
    public:
        /// @brief Constructs a WindowResizeEvent
        /// @param width the width of the window 
        /// @param height the height of the window 
        WindowResizeEvent(unsigned int width, unsigned int height)
        : m_width(width), m_height(height) {}

        unsigned int getWidth() const { return m_width; }
        unsigned int getHeight() const { return m_height; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_width << ", " << m_height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        unsigned int m_width, m_height;
    };

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppTickEvent : public Event
    {
    public:
        AppTickEvent() = default;

        EVENT_CLASS_TYPE(AppTick)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppUpdateEvent : public Event
    {
    public:
        AppUpdateEvent() = default;

        EVENT_CLASS_TYPE(AppUpdate)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class AppRenderEvent : public Event
    {
    public:
        AppRenderEvent() = default;

        EVENT_CLASS_TYPE(AppRender)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}