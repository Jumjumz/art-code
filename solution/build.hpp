#pragma once

#include <filesystem>

class Build {
  public:
    Build();

    void set_project_directory(const std::filesystem::path &dir);

    std::filesystem::path get_project_directory() const;

  private:
    std::filesystem::path project_directory;

    static constexpr std::string sln_ext = ".rcd";

    void create_project_content();
};
