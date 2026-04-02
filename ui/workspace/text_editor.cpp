#include "text_editor.hpp"

TextEditorWrapper::TextEditorWrapper() {
    // setup text editor
    this->editor.SetLanguageDefinition(
        TextEditor::LanguageDefinition::CPlusPlus());
    const std::vector<std::string> lines = {"#include <stdio.h>",
                                            "#include <artcode.h>",
                                            "",
                                            "int main() {",
                                            "\tstd::cout << \"Hello World!\";",
                                            "",
                                            "\treturn 0;",
                                            "}"};
    this->editor.SetTextLines(lines);
};

void TextEditorWrapper::render() {
    // load fonts
    ImGuiIO &io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromFileTTF(
        "assets/fonts/CascadiaMonoNFItalic.ttf", 22.0f);

    auto palette = TextEditor::GetDarkPalette();
    palette[(int)TextEditor::PaletteIndex::Background] = 0xFF1D1D1D;

    ImGui::PushFont(font);
    this->editor.SetPalette(palette);
    this->editor.Render("##artcode");
    ImGui::PopFont();
    ImGui::PopStyleVar();
};
