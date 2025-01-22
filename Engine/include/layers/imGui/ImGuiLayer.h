#pragma once

#include "layers/Layer.h"
#include "imgui.h"


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

    };

}