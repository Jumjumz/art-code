#include "build.hpp"

#include "json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

Build::Build() {};

bool Build::set_project_directory(const std::filesystem::path &dir) {
    this->project_directory = dir;

    return create_project_content();
};

void Build::create_config_dir() {
    if (!std::filesystem::exists(this->config_dir.parent_path())) {
        std::filesystem::create_directory(this->config_dir.parent_path());
    }

    // prepare to read and parse the projects.json file
    nlohmann::json js;
    if (std::filesystem::exists(this->config_dir)) {
        // read projects.json and parse
        std::ifstream read(this->config_dir);
        js = nlohmann::json::parse(read);
    } else {
        js["project_directory"] = nlohmann::json::array();
    }

    auto path = js["project_directory"];

    // check if new created path already exist.. skips if it is
    if (std::find(path.begin(), path.end(), this->project_directory.string()) ==
        path.end()) {
        // append new project directory
        js["project_directory"].push_back(this->project_directory);

        std::ofstream file(this->config_dir);
        file << js.dump(4);
    }
};

bool Build::create_project_content() {
    const std::vector<std::filesystem::path> content_directories = {
        this->project_directory / "build", this->project_directory / "shaders",
        this->project_directory / "components"};
    const auto file_name =
        this->project_directory.filename().string() + this->sln_ext;
    const auto solution_path = this->project_directory / file_name;

    // create sub directories
    if (!std::filesystem::exists(solution_path)) {
        // create and write solution file and main.cpp
        {
            std::vector<std::filesystem::path> project_content = {
                solution_path, this->project_directory / "main.cpp"};

            for (const auto &content : project_content) {
                if (content == this->project_directory / "main.cpp") {
                    const std::ofstream file(content);
                    // write main cpp init code
                    write_main_cpp(content);
                } else {
                    // create solution file
                    const std::ofstream file(content);
                    // write solution file
                    write_solution_file(content);
                }
            }
        }

        // create directories
        for (const auto &directory : content_directories) {
            if (directory == this->project_directory / "shaders") {
                std::filesystem::create_directory(directory);
                const std::ofstream shader_file(directory / "test.frag");
            }

            if (directory == this->project_directory / "components") {
                std::filesystem::create_directory(directory);
                std::vector<std::filesystem::path> comp_files = {
                    directory / "comp.hpp", directory / "comp.cpp"};
                for (const auto &comp : comp_files) {
                    const std::ofstream file(comp);
                }
            }

            std::cerr << directory << ": directory created" << std::endl;
        }

        // create config folder
        create_config_dir();

        return true;
    } else {
        std::cerr << solution_path << " project solution already exist"
                  << std::endl;
        return false;
    }
};

void Build::write_solution_file(const std::filesystem::path &solution_file) {
    // init json
    nlohmann::json js = {{"project_path", solution_file.parent_path()},
                         {"solution_file", solution_file.filename()}};

    // write
    std::ofstream write(solution_file);
    write << js.dump(4); // indent 4 spaces
};

void Build::write_main_cpp(const std::filesystem::path &main_cpp) {
    std::ofstream write(main_cpp);
    // write init code in strign literal
    write << R"(#include <iostream>

int main() {
    std::cout << "Hello World!";

    return 0;
};)";
};
