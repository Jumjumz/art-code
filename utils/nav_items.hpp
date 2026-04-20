#pragma once

#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

using NavMenuItems = const std::vector<std::tuple<std::string, std::string>>;
using NavMenu = std::tuple<const std::string, NavMenuItems>;

// All menu and menu items for main naviation
struct NavMainItems {
    static inline const std::vector<NavMenu> MENUS = {
        {"File", {{"New", "Ctrl+n"}, {"Open", "Ctrl+o"}}},
        {"Edit", {{"Artboard", "Ctrl+ar"}}},
        {"Window", {{"Panel", "Ctrl+p"}}},
        {"Help", {{"Documentation", "Ctrl+dd"}, {"About", ""}, {"Version", ""}}}};
};

struct NavBuildItems {
    static inline const NavMenuItems PANEL = {
        {"Debug", "Ctrl+d"}, {"Build", "Ctrl+b"}, {"Run", "Ctrl+r"}};
};

struct ProjectPath {
  public:
    static inline void set_project_path(const std::filesystem::path &proj_path) {
        ProjectPath::project_path = proj_path;
    };

    static inline std::filesystem::path get_project_path() {
        return ProjectPath::project_path;
    }

    static inline void set_solution_file(const std::filesystem::path &solution) {
        ProjectPath::solution_file = solution;
    }

    static inline std::filesystem::path get_solution_file() {
        return ProjectPath::solution_file;
    }

  private:
    static inline std::filesystem::path project_path;
    static inline std::filesystem::path solution_file;
};

struct CompilerResult {
  public:
    static inline void set_compiler_result(const std::string &compiler_result) {
        CompilerResult::compiler_result = compiler_result;
    }

    static inline std::string get_compiler_result() {
        return CompilerResult::compiler_result;
    }

    static inline void set_run_result(const std::string &run_result) {
        CompilerResult::run_result = run_result;
    }

    static inline std::string get_run_result() {
        return CompilerResult::run_result;
    }

  private:
    static inline std::string compiler_result;
    static inline std::string run_result;
};
