#pragma once

#include <filesystem>
#include <string>

class Build {
  public:
    Build();

    void get_project_directory(std::filesystem::path dir);

  private:
    std::string project_directory;
};
