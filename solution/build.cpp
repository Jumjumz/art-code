#include "build.hpp"

#include <filesystem>
#include <iostream>
#include <ostream>
#include <vector>

Build::Build() {};

// TODO: add a build system for artcode
void Build::get_project_directory(const std::filesystem::path &dir) {
    this->project_directory = dir;

    create_content_directory();

    std::cerr << this->project_directory.c_str() << std::endl;
};

void Build::create_content_directory() {
    const std::vector<std::filesystem::path> content_directories = {
        this->project_directory / "shaders",
        this->project_directory / "components"};

    for (const auto &directory : content_directories) {
        if (std::filesystem::create_directories(directory)) {
            std::cerr << directory << ": directory created" << std::endl;
        } else {
            std::cerr << directory << ": already exist" << std::endl;
        }
    }
};
