#include "build.hpp"

#include "json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

Build::Build() {};

bool Build::set_project_directory(const fs::path &dir, const glm::vec3 &artboard) {
    this->project_directory = dir;
    this->artboard_size = artboard;

    return create_project_content();
};

bool Build::create_project_content() {
    const std::vector<fs::path> content_directories = {
        this->project_directory / "build", this->project_directory / "shaders",
        this->project_directory / "components"};
    const auto file_name =
        this->project_directory.filename().string() + this->sln_ext;
    const auto solution_path = this->project_directory / file_name;

    // create sub directories
    if (!fs::exists(solution_path)) {
        // create and write solution file and main.cpp
        {
            std::vector<fs::path> project_content = {
                solution_path, this->project_directory / "main.cpp"};

            // create files
            for (const auto &content : project_content) {
                const std::ofstream file(content);
            }
            // write to respective files
            write_solution_file(project_content[0]);
            write_main_cpp(project_content[1]);
        }

        // create directories
        for (const auto &directory : content_directories) {
            fs::create_directory(directory);
            std::cerr << directory << ": directory created" << std::endl;
        }

        {
            // create files inside respective directories
            const auto shader = content_directories[1] / "artcode.frag";
            const std::ofstream shader_file(shader);

            const auto components_dir = content_directories[2];
            std::vector<fs::path> comp_files = {components_dir / "comp.hpp",
                                                components_dir / "comp.cpp"};
            for (const auto comp : components_dir) {
                const std::ofstream file(comp);
            }

            // write comp files
            write_comp_hpp(comp_files[0]);
            write_comp_cpp(comp_files[1]);
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

void Build::create_config_dir() {
    if (!fs::exists(this->config_dir.parent_path())) {
        fs::create_directory(this->config_dir.parent_path());
    }

    // prepare to read and parse the projects.json file
    nlohmann::json js;
    if (fs::exists(this->config_dir)) {
        // read projects.json and parse
        std::ifstream read(this->config_dir);
        js = nlohmann::json::parse(read);
    } else {
        js["project_directory"] = nlohmann::json::array();
    }

    nlohmann::json path = js["project_directory"];

    // check if new created path already exist.. skips if it is
    if (std::find(path.begin(), path.end(), this->project_directory.string()) ==
        path.end()) {
        // delete the first item if array size is 10
        if (path.size() == 10) {
            js["project_directory"].erase(js["project_directory"].begin());
        }

        // append new project directory
        js["project_directory"].push_back(this->project_directory);

        std::ofstream write(this->config_dir);
        write << js.dump(4);
    }
};

void Build::write_solution_file(const fs::path &solution_file) {
    // init json
    nlohmann::json js = {{"project_path", solution_file.parent_path()},
                         {"solution_file", solution_file.filename()},
                         {
                             "artboard_size",
                             {{"width", this->artboard_size.x},
                              {"height", this->artboard_size.y},
                              {"ppi", this->artboard_size.z}},
                         },
                         {"sources", {"main.cpp", "components/comp.cpp"}},
                         {"includes", nlohmann::json::array()}};

    // write
    std::ofstream write(solution_file);
    write << js.dump(4); // indent 4 spaces
};

void Build::write_main_cpp(const fs::path &main_cpp) {
    std::ofstream write(main_cpp);
    // write init code in strign literal
    write << R"(#include <artcode.hpp> 
#include "components/comp.hpp"

int main() {
    Component comp;
    Sources includes;
    // add source files to compile
    // -- list .cpp files here
    includes.add({});
    includes.build();

    return 0;
};)";
};

void Build::write_comp_cpp(const fs::path &comp) {
    std::ofstream write(comp);
    // write init code in strign literal
    write << R"(#include "comp.hpp"

Component::Component() {
    this->x = 10;
};)";
};

void Build::write_comp_hpp(const fs::path &comp) {
    std::ofstream write(comp);
    // write init code in strign literal
    write << R"(#pragma once

class Component {
    public:
      Component();

      int x;
    private:
};)";
};
