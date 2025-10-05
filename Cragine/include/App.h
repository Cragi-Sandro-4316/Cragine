#pragma once
#include "Window.h"
#include <memory>

namespace crg {
    class App {
    public:
        App();
        // ~App();

        App (const App&) = delete;
        App& operator= (const App&) = delete;

        void run();
    private:
        std::unique_ptr<Window> m_window;

    };
}
