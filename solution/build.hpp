#pragma once

#include <filesystem>

class Build {
  public:
    Build();

    bool set_project_directory(const std::filesystem::path &dir);

  private:
    std::filesystem::path project_directory;

    const std::filesystem::path config_dir =
        std::filesystem::path(getenv("HOME")) / ".config" / "artcode" /
        "projects.json";

    static constexpr std::string sln_ext = ".rcd";

    bool create_project_content();

    void create_config_dir();

    void write_solution_file(const std::filesystem::path &solution_file);

    void write_main_cpp(const std::filesystem::path &main_cpp);
};
