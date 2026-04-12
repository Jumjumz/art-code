#include "text_editor.hpp"
#include "nav_items.hpp"

TextEditorWrapper::TextEditorWrapper() {
    // setup project browser
    ImGui::FileBrowser file_browser(ImGuiFileBrowserFlags_CloseOnEsc |
                                    ImGuiFileBrowserFlags_CreateNewDir |
                                    ImGuiFileBrowserFlags_MultipleSelection);
    this->file_explorer = file_browser;

    this->file_explorer.SetTitle("Explorer");

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
    const ImVec2 content_size = ImGui::GetContentRegionAvail();
    const float panel_width = 40.0f;

    ImGui::BeginChild("##file_browser", ImVec2{panel_width, content_size.y},
                      false);
    for (const auto &[action, shortcut] : this->side_panel_contents) {
        if (ImGui::Button(action.c_str(), ImVec2{panel_width, panel_width})) {
            if (action == "Exp") {
                this->file_explorer.SetDirectory(ProjectPath::get_project_path());
                this->file_explorer.Open();
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine(); // render side by side

    // load font
    set_font();

    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
    ImGui::PushFont(this->font);
    this->editor.Render("##text-editor",
                        ImVec2{content_size.x - panel_width, content_size.y});
    ImGui::PopFont();
    ImGui::PopStyleVar();

    // TODO: have a fixed position for the model pop up
    this->file_explorer.SetWindowSize(100, content_size.y);
    // display file browser modal
    this->file_explorer.Display();

    if (this->file_explorer.HasSelected()) {
        this->file_explorer.ClearSelected();
    }
};

void TextEditorWrapper::set_font() {
    if (this->font == nullptr) {
        ImGuiIO &io = ImGui::GetIO();
        this->font = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/CascadiaMonoNFItalic.ttf", 22.0f);

        auto palette = TextEditor::GetDarkPalette();
        palette[(int)TextEditor::PaletteIndex::Background] = 0xFF1D1D1D;

        this->editor.SetPalette(palette);
    }
};
