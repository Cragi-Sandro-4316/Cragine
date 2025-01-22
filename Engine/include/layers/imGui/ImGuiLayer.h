#pragma once

#include "layers/Layer.h"

namespace Cragine {

    class ImGuiLayer : public Layer {
    public: 
        ImGuiLayer();
        ~ImGuiLayer();

        void onUpdate();
        void onEvent(Event& event);


    private: 

    };

}