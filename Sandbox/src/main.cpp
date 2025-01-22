#include <Cragine.h>
#include <cstdio>


// Example layer
class SampleLayer : public Cragine::Layer {
public:
    SampleLayer()
        : Layer("Example") {}


    void onUpdate() override {
        LOG_INFO("Example::Update");
    }

    void onEvent(Cragine::Event& event) override {
        LOG_INFO("{0}", event.toString());
    }

};

// Application class
class Sandbox : public Cragine::Application {
    public:
    Sandbox() {
        pushLayer(new SampleLayer());
        pushOverlay(new Cragine::ImGuiLayer());
    }

    ~Sandbox() {

    }
};

// Instantiates an Application
Cragine::Application* createApplication() {
    return new Sandbox();
}