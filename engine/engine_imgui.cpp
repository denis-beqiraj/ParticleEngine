#include "engine.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

ENG_API Eng::ImGuiEngine::ImGuiEngine(void* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void ENG_API Eng::ImGuiEngine::newFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Particle System");
}

bool ENG_API Eng::ImGuiEngine::newBar(std::string type, float& value, float min, float max)
{
    float old = value;
    ImGui::SliderFloat(type.c_str(), &value, min, max);
    if (old != value) {
        return true;
    }
    else {
        return false;
    }
}

void ENG_API Eng::ImGuiEngine::newText(std::string text)
{
    ImGui::Text(text.c_str());               // Display some text (you can use a format strings too)
}

bool ENG_API Eng::ImGuiEngine::newClick(std::string click, bool& value)
{
    float old = value;
    ImGui::Checkbox(click.c_str(), &value);
    if (old != value) {
        return true;
    }
    else {
        return false;
    }
}

bool ENG_API Eng::ImGuiEngine::newButton(std::string text)
{
    return ImGui::Button(text.c_str());
}

void ENG_API Eng::ImGuiEngine::render()
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
