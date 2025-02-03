#include "CraginePCH.h"
#include "layers/imGui/ImGuiLayer.h"

#include "core/Application.h"

#include "utils/Macros.h"
#include "utils/ImGuiUtils.h"
#include "input/Keycodes.h"

// TEMPORARY
#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace Cragine {
    
    ImGuiLayer::ImGuiLayer() 
        : Layer("ImGuiLayer") {}


    ImGuiLayer::~ImGuiLayer() {

    }


    void ImGuiLayer::onAttach() {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;


        ImGui_ImplOpenGL3_Init("#version 410");

    }

    void ImGuiLayer::onDetach() {

    }

    void ImGuiLayer::onUpdate() {
        
        ImGuiIO& io = ImGui::GetIO();
        
        Application& app = Application::get();

        io.DisplaySize = ImVec2(
            app.getWindow().getWidth(),
            app.getWindow().getHeight()
        );


        float time = (float)glfwGetTime();
        io.DeltaTime = deltaTime > 0.0 ? (time - deltaTime) : (1.0f / 60.0f);
        deltaTime = time;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    }

    void ImGuiLayer::onEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseButtonPressedEvent));
        dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseButtonReleasedEvent));
        dispatcher.dispatch<MouseMovedEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseMovedEvent));
        dispatcher.dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ImGuiLayer::onMouseScrolledEvent));

        dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyPressedEvent));
        dispatcher.dispatch<KeyTypedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyTypedEvent));
        dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::onKeyReleasedEvent));
        
        dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(ImGuiLayer::onWindowResizeEvent));


    }

    bool ImGuiLayer::onMouseButtonPressedEvent(MouseButtonPressedEvent& e) {

        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[e.getMouseButton()] = true;

        return false;
    }

    bool ImGuiLayer::onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e) {
        
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[e.getMouseButton()] = false;

        return false;
    }
    
    bool ImGuiLayer::onMouseMovedEvent(MouseMovedEvent& e) {
        
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2(e.getX(), e.getY());

        return false;
    }
    
    bool ImGuiLayer::onMouseScrolledEvent(MouseScrolledEvent& e) {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheelH += e.getXOffset();
        io.MouseWheel += e.getYOffset();

        return false;
    }
    
    bool ImGuiLayer::onKeyPressedEvent(KeyPressedEvent& e) {
        ImGuiIO& io = ImGui::GetIO();
        
        ImGuiKey key = mapCragineKeyToImGuiKey(e.getKeyCode());
        io.AddKeyEvent(key, true); // `true` means key is pressed
        
        io.AddKeyEvent (
            ImGuiKey_LeftCtrl, 
            e.getKeyCode() == CRAGINE_KEY_LEFT_CONTROL || 
            e.getKeyCode() == CRAGINE_KEY_RIGHT_CONTROL
        );

        io.AddKeyEvent(
            ImGuiKey_LeftShift, 
            e.getKeyCode() == CRAGINE_KEY_LEFT_SHIFT || 
            e.getKeyCode() == CRAGINE_KEY_RIGHT_SHIFT
        );
        
        io.AddKeyEvent(
            ImGuiKey_LeftAlt, 
            e.getKeyCode() == CRAGINE_KEY_LEFT_ALT || 
            e.getKeyCode() == CRAGINE_KEY_RIGHT_ALT
        );

        io.AddKeyEvent(
            ImGuiKey_LeftSuper, 
            e.getKeyCode() == CRAGINE_KEY_LEFT_SUPER || 
            e.getKeyCode() == CRAGINE_KEY_RIGHT_SUPER
        );

        return false;
    }
    
    bool ImGuiLayer::onKeyReleasedEvent(KeyReleasedEvent& e) {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiKey key = mapCragineKeyToImGuiKey(e.getKeyCode());

        io.AddKeyEvent(key, false); // `false` means key is released
        return false;
    }   

    bool ImGuiLayer::onKeyTypedEvent(KeyTypedEvent& e) {
        ImGuiIO& io = ImGui::GetIO();
        int keycode = e.getKeyCode();

        if (keycode > 0 && keycode < 0x10000)
            io.AddInputCharacter((unsigned short) keycode);
        
        return false;
    }



    bool ImGuiLayer::onWindowResizeEvent(WindowResizeEvent& e) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(e.getWidth(), e.getHeight());
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        glViewport(0, 0, e.getWidth(), e.getHeight());


        return false;
    }

    
}