#pragma once

#include <filesystem>

class Build {
  public:
    Build();

    void get_project_directory(const std::filesystem::path &dir);

  private:
    std::filesystem::path project_directory;

    static constexpr std::string sln_ext = ".rcd";

    void create_project_content();
};
