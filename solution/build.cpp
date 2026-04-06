#include "build.hpp"

#include <iostream>

Build::Build() { this->project_directory = ""; };

// TODO: add a build system for artcode

void Build::get_project_directory(std::filesystem::path dir) {
    this->project_directory = dir.string();

    std::cerr << this->project_directory << std::endl;
};
