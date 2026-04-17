#pragma once

#include <filesystem>
#include <string>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    std::string executable_files(const std::filesystem::path &project_dir);

    // C for compile, R for Run
    enum class Flags { C, R };

    std::string execute(const Flags &flag);
};
