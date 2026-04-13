#pragma once

#include "TextEditor.h"
#include <imfilebrowser.h>

class TextEditorWrapper {
  public:
    TextEditorWrapper();

    void render();

  private:
    TextEditor editor;

    ImGui::FileBrowser file_explorer;

    std::filesystem::path selected_file;

    ImFont *font = nullptr;

    void set_font();

    void read_code();

    void save_written_code();

    const std::vector<std::tuple<std::string, std::string>> side_panel_contents = {
        {"Exp", "Ctrl+x"}};
};
