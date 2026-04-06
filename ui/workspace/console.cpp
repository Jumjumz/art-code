#include "console.hpp"

Console::Console() {};

void Console::render() {
    if (this->font == nullptr) {
        ImGuiIO &io = ImGui::GetIO();
        this->font = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/MapleMono-Italic.ttf", 18.0f);
    }
    ImGui::PushFont(this->font);
    ImGui::Text("Hello World!");
    ImGui::PopFont();
};
