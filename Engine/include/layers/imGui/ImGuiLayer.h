#pragma once

#include "layers/Layer.h"
#include "imgui.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"


namespace Cragine {

    class ImGuiLayer : public Layer {
    public: 
        ImGuiLayer();
        ~ImGuiLayer();

        virtual void onAttach() override;
        virtual void onDetach() override;
        virtual void onImGuiRender() override;

        // void onEvent(Event& event);
        
        void begin();
        void end();

    private: 
        float deltaTime = 0.0f;

        // bool onMouseButtonPressedEvent(MouseButtonPressedEvent& e);
        // bool onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
        // bool onMouseMovedEvent(MouseMovedEvent& e);
        // bool onMouseScrolledEvent(MouseScrolledEvent& e);
        
        // bool onKeyTypedEvent(KeyTypedEvent& e);
        // bool onKeyPressedEvent(KeyPressedEvent& e);
        // bool onKeyReleasedEvent(KeyReleasedEvent& e);
        
        // bool onWindowResizeEvent(WindowResizeEvent& e);

    };

}