#pragma once

#include "Core.h"
#include "Events/Event.h"

namespace Cragine {
    class Application {
    public:
        Application();
        virtual ~Application();

        void run();

    };

    /// @brief To be defined in the client
    Application* createApplication();
}