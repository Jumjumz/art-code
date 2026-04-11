#pragma once

#include "build.hpp"
#include "imgui.h"
#include <imfilebrowser.h>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    Build build;

    ImGui::FileBrowser file_explorer;
};
