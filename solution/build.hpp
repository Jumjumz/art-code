#pragma once

#include <filesystem>

class Build {
  public:
    Build();

    bool set_project_directory(const std::filesystem::path &dir);

    std::filesystem::path get_project_directory() const;

  private:
    std::filesystem::path project_directory;

    const std::filesystem::path config_dir =
        std::filesystem::path(getenv("HOME")) / ".config" / "artcode" /
        "projects.json";

    static constexpr std::string sln_ext = ".rcd";

    bool create_project_content();

    void create_config_dir();

    void write_solution_file(const std::filesystem::path &solution_file);
};
