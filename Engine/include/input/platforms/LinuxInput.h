#pragma once

#include "input/Input.h"

namespace Cragine {

    class LinuxInput : public Input {
    protected: 
        virtual bool isKeyPressedImpl(int keycode) override;

        virtual bool isMousePressedImpl(int button) override;
        virtual std::pair<float, float> getMousePositionImpl() override;
        virtual float getMouseXImpl() override;
        virtual float getMouseYImpl() override;
    };

}