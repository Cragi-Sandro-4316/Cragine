#pragma once

#include "Event.h"

namespace Cragine {
    class KeyEvent : public Event {
    public:     
        inline int getKeyCode() const { return m_keyCode; }
        
        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

    protected: 
        KeyEvent(int keycode) : m_keyCode(keycode) {}

        int m_keyCode;
    };



    class KeyPressedEvent : public KeyEvent {
    public: 
        KeyPressedEvent(int keycode, int repeatCount) 
            : KeyEvent(keycode), m_repeatCount(repeatCount) {}

        inline int getREpeatCount() const { return m_repeatCount; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << m_keyCode << " (" << m_repeatCount <<" repeats)";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed);

    private: 
        int m_repeatCount;
    };


    class KeyReleasedEvent : public KeyEvent {
    public: 
        KeyReleasedEvent(int m_keyCode) 
            : KeyEvent(m_keyCode) {}

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << m_keyCode;
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };

    class KeyTypedEvent : public KeyEvent {
    public:
        KeyTypedEvent(int m_keyCode)
            : KeyEvent(m_keyCode) {}

        

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyTypedEvent: " << m_keyCode;
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyTyped)
    };


}
