#include "CraginePCH.h"
#include "layers/imGui/ImGuiLayer.h"

#include "core/Application.h"

#include "platform/OpenGL/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    }

    void ImGuiLayer::onEvent(Event& event) {

    }


}