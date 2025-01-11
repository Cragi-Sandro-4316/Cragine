#include <Cragine.h>
#include <cstdio>

// Application class
class Sandbox : public Cragine::Application {
    public:
    Sandbox() {

    }

    ~Sandbox() {

    }
};

// Instantiates an Application
Cragine::Application* createApplication() {
    return new Sandbox();
}