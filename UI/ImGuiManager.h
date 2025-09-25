#pragma once
#include "../Start.h"

class ImGuiManager {
public:
    ImGuiManager() = default;

    // singleon
    static ImGuiManager& get_singleton() {
        static ImGuiManager singleton = ImGuiManager();
        return singleton;
    }

    void imgui_new_frame(bool &show_demo_window) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        show_all_components(show_demo_window);
    }

    void render(const VkCommandBuffer &command_buffer) {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    }

    void init_basic_config() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        ImGui::StyleColorsDark();
    }



private:
    void show_all_components(bool &show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open", "Ctrl+O")) { /* 打开文件操作 */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* 保存文件操作 */ }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* 撤销操作 */ }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
};