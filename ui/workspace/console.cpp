#include "console.hpp"
#include "imgui.h"

Console::Console() {

};

void Console::render() {
    ImGuiIO &io = ImGui::GetIO();
    auto font =
        io.Fonts->AddFontFromFileTTF("assets/fonts/MapleMono-Italic.ttf", 18.0f);

    ImGui::PushFont(font);
    ImGui::Text("Hello World!");
    ImGui::PopFont();
};
