#include "build.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

Build::Build() {};

// TODO: add a build system for artcode
void Build::get_project_directory(const std::filesystem::path &dir) {
    this->project_directory = dir;

    create_project_content();
};

void Build::create_project_content() {
    const std::vector<std::filesystem::path> content_directories = {
        this->project_directory / "shaders",
        this->project_directory / "components"};
    const auto file_name =
        this->project_directory.filename().string() + this->sln_ext;

    if (!std::filesystem::exists(file_name)) {
        // create solution file
        {
            std::vector<std::filesystem::path> project_content = {
                file_name, this->project_directory / "main.cpp"};

            for (const auto &content : project_content) {
                const std::ofstream file(content);
            }
        }

        // create sub directories
        for (const auto &directory : content_directories) {
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
        }
    } else {
        std::cerr << "Project solution already exists: " << file_name
                  << std::endl;
    }
};
