#pragma once

#include <string>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    // C for compile, R for Run
    enum class Flags { C, R };

    std::string execute(const Flags &flag);
};
