#include "text_editor.hpp"
#include "nav_items.hpp"
#include "vk_types.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

TextEditorWrapper::TextEditorWrapper() {
    // setup project browser
    ImGui::FileBrowser file_browser(
        ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_CreateNewDir |
        ImGuiFileBrowserFlags_MultipleSelection | ImGuiFileBrowserFlags_NoModal |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    this->file_explorer = file_browser;

    this->file_explorer.SetTitle("Explorer");

    // setup text editor
    this->editor.SetLanguageDefinition(
        TextEditor::LanguageDefinition::CPlusPlus());
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

    // FIXME: this doesnt work as intended..
    if (this->file_explorer.IsOpened()) {
        const auto viewport = ImGui::GetMainViewport();
        const auto work_size = viewport->WorkSize;
        const auto work_pos = viewport->WorkPos;

        this->file_explorer.SetWindowSize(100, int(content_size.y + 40.0f));
        ImGui::SetNextWindowPos(ImVec2{work_size.x * 0.4f, work_pos.y},
                                ImGuiCond_Always);
    }

    // display file browser modal
    this->file_explorer.Display();

    if (this->file_explorer.HasSelected()) {
        // pass the selected file
        this->selected_file = this->file_explorer.GetSelected();

        // display the selected file to text editor
        read_code();
        this->file_explorer.ClearSelected();
    }

    // save written code
    save_written_code();
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

void TextEditorWrapper::read_code() {
    std::ifstream read(this->selected_file);
    // get contents of the file
    std::string content((std::istreambuf_iterator<char>(read)),
                        std::istreambuf_iterator<char>());

    if (content.back() == '\n' || content.back() == '\r' ||
        content.back() == '\t') {
        content.pop_back();
    }

    this->editor.SetText(content);

    read.close();
};

void TextEditorWrapper::save_written_code() {
    // update the file
    if (TextEditorUtils::file_save) {
        std::ofstream write(this->selected_file);

        if (write.is_open()) {
            // read the entire code as string
            write << this->editor.GetText();
            write.close();

            std::cerr << "File saved!" << std::endl;
        } else {
            std::cerr << "Cannot write on this file" << std::endl;
        }

        // read the newly updated code
        read_code();

        // rest state of file save
        TextEditorUtils::file_save = false;
    }
};
