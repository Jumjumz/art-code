#pragma once

#include "build.hpp"

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    Build build;
};
