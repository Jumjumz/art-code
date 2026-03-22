#include "text_editor.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <string>

TextEditor::TextEditor() {};

void TextEditor::render() {
    auto *viewport = ImGui::GetMainViewport();
    auto work_size = viewport->WorkSize;
    auto work_pos = viewport->WorkPos;

    float width = work_size.x * 0.4f; // 40%
    float height = work_size.y;
    float pos_x = work_pos.x + (work_size.x * 0.6f); // position to the right
    float pos_y = work_pos.y;
    std::string data = ""; // test

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("##text-editor-begin", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);
    ImGui::InputTextMultiline("##test-editor-main", &data, ImVec2{width, height});
    ImGui::End();
    ImGui::PopStyleVar();

    // TODO::
    //  create a text editor using Imgui::InputText
};
