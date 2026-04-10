#include "build.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

Build::Build() {};

// TODO: add a build system for artcode
void Build::set_project_directory(const std::filesystem::path &dir) {
    this->project_directory = dir;

    create_project_content();
};

// TODO: instead of return this->project_directory.. read the .rcd file which contains the project dir
std::filesystem::path Build::get_project_directory() const {
    return this->project_directory;
};

// TODO: add functions that will read and write the .rcd file

void Build::create_project_content() {
    const std::vector<std::filesystem::path> content_directories = {
        this->project_directory / "shaders",
        this->project_directory / "components"};
    const auto file_name =
        this->project_directory.filename().string() + this->sln_ext;
    const auto solution_path = this->project_directory / file_name;

    // create sub directories
    if (!std::filesystem::exists(solution_path)) {
        // create solution file
        {
            std::vector<std::filesystem::path> project_content = {
                solution_path, this->project_directory / "main.cpp"};

            for (const auto &content : project_content) {
                const std::ofstream file(content);
            }
        }

        for (const auto &directory : content_directories) {
            if (std::filesystem::create_directories(directory)) {
                if (directory == this->project_directory / "shaders") {
                    const std::ofstream shader_file(directory / "test.frag");
                }

                if (directory == this->project_directory / "components") {
                    std::vector<std::filesystem::path> comp_files = {
                        directory / "comp.hpp", directory / "comp.cpp"};
                    for (const auto &comp : comp_files) {
                        const std::ofstream file(comp);
                    }
                }

                std::cerr << directory << ": directory created" << std::endl;
            } else {
                std::cerr << directory << ": directory already exist"
                          << std::endl;
                break;
            }
        }
    } else {
        std::cerr << solution_path << " project solution already exist"
                  << std::endl;
    }
};
