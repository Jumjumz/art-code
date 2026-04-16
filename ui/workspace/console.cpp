#include "console.hpp"
#include "imgui.h"
#include "nav_items.hpp"

Console::Console() {};

void Console::render() {
    if (this->font == nullptr) {
        ImGuiIO &io = ImGui::GetIO();
        this->font = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/MapleMono-Italic.ttf", 18.0f);
    }
    const auto project_path = ProjectPath::get_project_path() / "$ ";
    const auto compiler_result = CompilerResult::get_compiler_result();
    const auto run_result = CompilerResult::get_run_result();

    ImGui::PushFont(this->font);
    ImGui::TextUnformatted(project_path.c_str());
    // TODO: should not check if empty, but base if compiler returned an error
    if (!run_result.empty()) {
        ImGui::Text("%s", run_result.c_str());
    } else {
        ImGui::Text("%s", compiler_result.c_str());
    }
    ImGui::PopFont();
};
