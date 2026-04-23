#pragma once

#include <string>
#include <vector>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    std::string executable_files() const;

    // C for compile, R for Run
    enum class Flags { C, R };

    void add_includes(const std::vector<std::string> &selected) const;

    std::string execute(const Flags &flag) const;
};
