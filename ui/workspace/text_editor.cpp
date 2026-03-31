#include "text_editor.hpp"

TextEditorWrapper::TextEditorWrapper() {
    // setup text editor
    this->editor.SetLanguageDefinition(
        TextEditor::LanguageDefinition::CPlusPlus());
    this->editor.SetText("// Write code below...");
};

void TextEditorWrapper::render() {
    auto *viewport = ImGui::GetMainViewport();
    auto work_size = viewport->WorkSize;
    auto work_pos = viewport->WorkPos;

    const float width = work_size.x * 0.4f; // 40%
    const float height = work_size.y;
    const float pos_x = work_size.x - width; // absolute position to the right
    const float pos_y = work_pos.y;

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("##text-editor-begin", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    this->editor.Render("ArtCode code");
    ImGui::End();
    ImGui::PopStyleVar();
};
