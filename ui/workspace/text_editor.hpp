#pragma once

#include "TextEditor.h"
#include <imfilebrowser.h>

class TextEditorWrapper {
  public:
    TextEditorWrapper();

    void render();

  private:
    static constexpr size_t TAB_ITEMS_NUM = 5;

    TextEditor editor;

    ImGui::FileBrowser file_explorer = ImGui::FileBrowser(
        ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_CreateNewDir |
        ImGuiFileBrowserFlags_MultipleSelection | ImGuiFileBrowserFlags_NoModal |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    std::filesystem::path selected_file;

    ImFont* font = nullptr;

    std::vector<std::filesystem::path> tabs = {};

    void set_font();

    void set_language();

    void read_code();

    void save_written_code();

    const std::vector<std::tuple<std::string, std::string>> side_panel_contents = {
        {"Exp", "Ctrl+x"}};
};
