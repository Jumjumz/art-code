#include "text_editor.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <string>

TextEditor::TextEditor() {};

void TextEditor::text_editor_render() {
    auto *viewport = ImGui::GetMainViewport();
    auto work_size = viewport->WorkSize;
    auto work_pos = viewport->WorkPos;

    auto width = work_size.x * 0.4f; // 40%
    auto height = work_size.y;
    auto pos_x = work_pos.x + (work_size.x * 0.6f); // position to the right
    auto pos_y = work_pos.y;
    std::string data = ""; // test

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // removes padding for begin component
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##text-editor-begin", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    ImGui::InputTextMultiline("##test-editor-main", &data, ImVec2{width, height});
    ImGui::End();
    ImGui::PopStyleVar();
};
