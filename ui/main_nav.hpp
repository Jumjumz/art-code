#pragma once

#include "imgui.h"
#include <imfilebrowser.h>

class MainNavigation {
  public:
    MainNavigation();

    void render();

  private:
    ImGui::FileBrowser file_dialog;
};
