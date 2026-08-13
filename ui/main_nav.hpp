#pragma once

#include "imgui.h"
#include <cstdlib>
#include <imfilebrowser.h>

class MainNavigation {
  public:
    MainNavigation();

    void render();

  private:
    const char*        home_dir = getenv("HOME");
    ImGui::FileBrowser file_dialog;
};
