#pragma once

#include "imgui.h"

class Console {
  public:
    void render();

  private:
    ImFont* font = nullptr;
};
