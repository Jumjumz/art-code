#pragma once

#include "imgui.h"

class Console {
  public:
    Console();

    void render();

  private:
    ImFont *font = nullptr;
};
