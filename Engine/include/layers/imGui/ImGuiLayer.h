#pragma once

#include "layers/Layer.h"
#include "imgui.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "imgui_impl_opengl3.h"


namespace Cragine {

    class ImGuiLayer : public Layer {
    public: 
        ImGuiLayer();
        ~ImGuiLayer();

        void onAttach();
        void onDetach();

        void onUpdate();
        void onEvent(Event& event);



    private: 
        float deltaTime = 0.0f;

        bool onMouseButtonPressedEvent(MouseButtonPressedEvent& e);
        bool onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
        bool onMouseMovedEvent(MouseMovedEvent& e);
        bool onMouseScrolledEvent(MouseScrolledEvent& e);
        
        bool onKeyTypedEvent(KeyTypedEvent& e);
        bool onKeyPressedEvent(KeyPressedEvent& e);
        bool onKeyReleasedEvent(KeyReleasedEvent& e);
        
        bool onWindowResizeEvent(WindowResizeEvent& e);


    };

}