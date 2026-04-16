#pragma once

#include <string>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    void build_artcode();

    void run_executable();

    enum class Flags { C, R };

    std::string execute(const Flags &flag);
};
