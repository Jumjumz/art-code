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

    bool create_project_content() const;

    void create_config_dir() const;

    void write_solution_file(const fs::path &solution_file) const;

    void write_main_cpp(const fs::path &main_cpp) const;

    void write_comp_cpp(const fs::path &comp) const;

    void write_comp_hpp(const fs::path &comp) const;

    void write_shader(const fs::path &shader) const;
};
