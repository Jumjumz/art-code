#pragma once

#include <filesystem>
#include <glm/glm.hpp>

class Build {
  public:
    Build();

    static constexpr std::string sln_ext = ".rcd";

    bool set_project_directory(const std::filesystem::path &dir,
                               const glm::vec3 &artboard);

  private:
    std::filesystem::path project_directory;

    glm::vec3 artboard_size;

    const std::filesystem::path config_dir =
        std::filesystem::path(getenv("HOME")) / ".config" / "artcode" /
        "projects.json";

    bool create_project_content();

    void create_config_dir();

    void write_solution_file(const std::filesystem::path &solution_file);

    void write_main_cpp(const std::filesystem::path &main_cpp);

    void write_comp_cpp(const std::filesystem::path &comp);

    void write_comp_hpp(const std::filesystem::path &comp);
};
