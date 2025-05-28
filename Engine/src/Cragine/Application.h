#pragma once

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