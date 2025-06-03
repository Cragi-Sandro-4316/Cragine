#pragma once

#include "../Core.h"

#include "../CraginePCH.h"

// #include <string>
// #include <functional>
// #include <sstream>

namespace Cragine {

    // Events are currently blocking.
    // Any event dispatched will be handled immediatly.
    // In the future events will be buffered.

    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };


    /// Bitfield that identifies the Event Category
    enum EventCategory {
        None = 0,
        EventCategoryApplication    = BIT(0),
        EventCategoryInput          = BIT(1),
        EventCategoryKeyboard       = BIT(2),
        EventCategoryMouse          = BIT(3),
        EventCategoryMouseButton    = BIT(4)
    };


    // ----------------------------------------- MACRO DEFINITIONS ----------------------------------------------
#define EVENT_CLASS_TYPE(type) \
    static EventType getStaticType() { return EventType::type; }\
    EventType getEventType() const override { return getStaticType(); }\
    virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }
// -------------------------------------------------------------------------------------------------------------

    class Event {
    public:
        virtual ~Event() = default;

        // If an event is handled or not
        bool m_handled = false;

        virtual EventType getEventType() const = 0;
        virtual const char* getName() const = 0;
        virtual int getCategoryFlags() const = 0;
        virtual std::string ToString() const { return getName(); }

        bool isInCategory(EventCategory category)
        {
            return getCategoryFlags() & category;
        }

    };


    class EventDispatcher {
        template<typename T>
        using EventFn = std::function<bool(T&)>;
    public:

        EventDispatcher(Event& event)
            : m_event(event) {}

        template<typename T>
        bool dispatch(EventFn<T> func) {
            if (m_event.getEventType() == T::getStaticType()) {
                m_event.m_handled = func(*static_cast<T*>(&m_event));
                return true;
            }
            return false;

        }

    private:
        Event& m_event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.ToString();
    }

}