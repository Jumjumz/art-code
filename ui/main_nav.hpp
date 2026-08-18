#pragma once

#include "imgui.h"
#include <cstdlib>
#include <imfilebrowser.h>

class MainNavigation {
  public:
    void render();

  private:
    const char*        home_dir    = getenv("HOME");
    ImGui::FileBrowser file_dialog = ImGui::FileBrowser{
        ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir,
        home_dir};
};
