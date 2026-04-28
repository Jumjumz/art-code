#include "console.hpp"
#include "nav_items.hpp"

Console::Console() {};

void Console::render() {
    if (this->font == nullptr) {
        ImGuiIO &io = ImGui::GetIO();
        this->font = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/MapleMono-Italic.ttf", 18.0f);
    }
    const auto project_path = ProjectPath::get_project_path() / "$ ";
    const auto execute_result = ExecuteResult::get_result();
    const auto exit_code = ExecuteResult::get_exit_code();

    ImGui::PushFont(this->font);
    ImGui::TextUnformatted(project_path.c_str());
    // 0 != compilation errors/runtime errors (seg faults)
    if (exit_code != 0) {
        ImGui::Text("%s", execute_result.c_str());
    } else {
        ImGui::Text("%s", execute_result.c_str());
    }
    ImGui::PopFont();
};
