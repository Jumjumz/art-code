#pragma once

#include <filesystem>
#include <glm/glm.hpp>

namespace fs = std::filesystem;

class Build {
  public:
    Build();

    static constexpr std::string sln_ext = ".rcd";

    bool set_project_directory(const fs::path &dir, const glm::vec3 &artboard);

  private:
    fs::path project_directory;

    glm::vec3 artboard_size;

    const fs::path config_dir =
        fs::path(getenv("HOME")) / ".config" / "artcode" / "projects.json";

    bool create_project_content();

    void create_config_dir();

    void write_solution_file(const fs::path &solution_file);

    void write_main_cpp(const fs::path &main_cpp);

    void write_comp_cpp(const fs::path &comp);

    void write_comp_hpp(const fs::path &comp);
};
