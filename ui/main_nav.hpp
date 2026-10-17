#pragma once

#include "imgui.h"
#include <cstdlib>
#include <imfilebrowser.h>

class MainNavigation {
  public:
    void render();

  private:
    #ifdef _WIN32
        const char* home_dir = getenv("USERPROFILE");
    #else
        const char* home_dir = getenv("HOME");
    #endif
    ImGui::FileBrowser file_dialog = ImGui::FileBrowser{
        ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir,
        home_dir};
};
