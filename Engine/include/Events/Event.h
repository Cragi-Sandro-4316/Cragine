#pragma once
#include "CraginePCH.h"

#define BIT(x) (1 << x)

namespace Cragine {

    /// the Type of the event
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
// ----------------------------------------------------------------------------------------------------------

    class Event {
    public:
        virtual ~Event() = default;

        // If an event is handled or not
        bool handled = false;

        virtual EventType getEventType() const = 0;
        virtual const char* getName() const = 0;
        virtual int getCategoryFlags() const = 0;
        virtual std::string toString() const { return getName(); }

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
            : event(event) {}

        template<typename T>
        bool dispatch(EventFn<T> func) {
            if (event.getEventType() == T::getStaticType()) {
                event.handled = func(*static_cast<T*>(&event));
                return true;
            }
            return false;

        }

    private:
        Event& event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.toString();
    }

} 