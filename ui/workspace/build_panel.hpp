#pragma once

#include <string>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    std::string executable_files();

    // C for compile, R for Run
    enum class Flags { C, R };

    std::string execute(const Flags &flag);
};
