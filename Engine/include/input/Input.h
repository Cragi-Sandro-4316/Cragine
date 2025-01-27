#pragma once


namespace Cragine {

    class Input {
    public: 
        inline static bool isKeyPressed(int keycode) { return instance->isKeyPressedImpl(keycode); }

        inline static bool isMouseButtonPressed(int button) { return instance->isMousePressedImpl(button); }
        inline static std::pair<float, float> getMousePosition() { return instance->getMousePositionImpl(); }

        inline static float getMouseX() { return instance->getMouseXImpl(); }
        inline static float getMouseY() { return instance->getMouseYImpl(); }


    protected: 
        virtual bool isKeyPressedImpl(int keycode) = 0;

        virtual bool isMousePressedImpl(int button) = 0;
        virtual std::pair<float, float> getMousePositionImpl() = 0;
    
        virtual float getMouseXImpl() = 0;
        virtual float getMouseYImpl() = 0;


    private: 
        static Input* instance;
    };
}