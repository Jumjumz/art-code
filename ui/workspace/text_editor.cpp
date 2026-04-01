#include "text_editor.hpp"

TextEditorWrapper::TextEditorWrapper() {
    // setup text editor
    this->editor.SetLanguageDefinition(
        TextEditor::LanguageDefinition::CPlusPlus());
    const std::vector<std::string> lines = {"#include <stdio.h>",
                                            "",
                                            "int main() {",
                                            "\tstd::cout << \"Hello World!\";",
                                            "",
                                            "\treturn 0;",
                                            "}"};
    this->editor.SetTextLines(lines);
};

void TextEditorWrapper::render() {
    auto *viewport = ImGui::GetMainViewport();
    auto work_size = viewport->WorkSize;
    auto work_pos = viewport->WorkPos;

    const float width = work_size.x * 0.4f; // 40%
    const float height = work_size.y;
    const float pos_x = work_size.x - width; // absolute position to the right
    const float pos_y = work_pos.y;

    // load fonts
    ImGuiIO &io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromFileTTF(
        "assets/fonts/CascadiaMonoNFItalic.ttf", 24.0f);

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("##text-editor-begin", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(font);
    this->editor.Render("##artcode");
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
};
