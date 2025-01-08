#include <Cragine.h>

class Sandbox : public Cragine::Application {
    public:
    Sandbox() {

    }

    ~Sandbox() {

    }
};

Cragine::Application* CreateApplication() {
    return new Sandbox();
}