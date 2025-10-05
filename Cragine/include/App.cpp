#include "App.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace crg {

    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    App::App() {
        m_window = std::make_unique<Window>(
            WIDTH,
            HEIGHT,
            "Pipa Engine"
        );
    }

    void App::run() {
        while(!glfwWindowShouldClose(m_window->getGlfwWindow())) {
            glfwPollEvents();
            std::cout<<"pipa\n";
        }
        std::cout<<"Uscito con successo"<<std::endl;
    }
}
