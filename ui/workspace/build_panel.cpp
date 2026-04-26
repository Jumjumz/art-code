#include "build_panel.hpp"
#include "imgui.h"
#include "json.hpp"
#include "nav_items.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

BuildPanel::BuildPanel() {};

void BuildPanel::render() {
    const auto panel_size = ImGui::GetContentRegionAvail();
    const float width = 100.0f;
    const float height = 20.0f;
    bool show_adding_includes = false;

    for (const auto &[action, shortcut] : NavBuildItems::PANEL) {
        ImGui::SetCursorPosY((panel_size.y - height) / 2.0f);
        if (ImGui::Button(action.c_str(), ImVec2{width, height})) {
            if (action == "Includes") {
                // auto add .cpp files in proj dir to solution file
                show_adding_includes = true;
                add_includes();
            }

            if (action == "Build") {
                compile();
            }

            if (action == "Run") {
                if (!this->project_compiled) {
                    compile();
                }
                const auto result = create_cmd(BuildPanel::Flags::R);
                CompilerResult::set_run_result(result);
                this->project_compiled = false;
            }
        }
        ImGui::SameLine();
    }

    if (show_adding_includes) {
        ImGui::Text("Adding includes...");
        show_adding_includes = false;
    }
};

void BuildPanel::compile() {
    this->project_compiled = true;
    const auto result = create_cmd(BuildPanel::Flags::C);
    CompilerResult::set_compiler_result(result);
    // prevent persistent data of run result when recompiling
    CompilerResult::set_run_result("");
};

void BuildPanel::add_includes() const {
    auto solution_file = ProjectPath::get_solution_file();

    nlohmann::json js;
    {
        std::ifstream read(solution_file);
        js = nlohmann::json::parse(read);
    }

    auto includes = nlohmann::json::array();
    // search for new .cpp files in project dir
    for (const auto &file :
         fs::recursive_directory_iterator(solution_file.parent_path())) {
        const auto file_path = file.path();
        if (file_path.extension() == ".cpp") {
            const auto relative_path =
                fs::relative(file_path, solution_file.parent_path());
            // exclude the known files
            if (relative_path == "components/comp.cpp" ||
                relative_path == "main.cpp") {
                continue;
            } else {
                includes.push_back(relative_path);
            }
        }
    }

    // replace entire includes everytime this function runs
    js["includes"] = includes;

    std::ofstream write(solution_file);
    write << js.dump(4);
};

std::string BuildPanel::shader_files() const {
    const auto shader_file = ProjectPath::get_solution_file();
    std::ifstream read(shader_file);
    nlohmann::json js;
    js = nlohmann::json::parse(read);
    read.close();

    return js["shaders"].get<std::string>();
};

std::string BuildPanel::executable_files() const {
    // read solution file
    std::vector<std::string> executables;
    {
        const auto solution_file = ProjectPath::get_solution_file();
        std::ifstream read(solution_file);

        nlohmann::json js;
        js = nlohmann::json::parse(read);
        read.close();

        const auto includes = js["includes"].get<std::vector<std::string>>();
        executables = js["sources"].get<std::vector<std::string>>();
        // append includes if not empty
        if (!includes.empty()) {
            executables.insert(executables.end(), includes.begin(),
                               includes.end());
        }
    }

    std::string source;
    for (const auto &dir : executables) {
        source += dir + " "; // add space at the end of each path
    }

    return source;
};

std::string BuildPanel::create_cmd(const BuildPanel::Flags &flag) {
    std::string cmd;
    // execute and use gcc compiler
    {
        const auto project_dir = ProjectPath::get_project_path();
        const std::string build = project_dir / "build/artcode";

        switch (flag) {
        case BuildPanel::Flags::C: {
            // compile c++ code
            {
                const auto executables = executable_files();
                // change to project dir before compiling
                cmd = "cd " + project_dir.string();
                cmd += " && ";
                // access the api dir to locate artcode.hpp library
                fs::path exe_dir = fs::canonical("/proc/self/exe").parent_path();
                fs::path api_dir = exe_dir / "api";

                // compile c++
                cmd += "g++ -std=c++20 " + executables;
                // compile main with artcode shared lib
                cmd += "-I" + api_dir.string() + " ";
                cmd += "-L" + api_dir.string() + " ";
                cmd += "-lapi ";
                cmd += "-Wl,-rpath," + api_dir.string() + " ";
                cmd += "-o " + build + " 2>&1";
            }
            // compile shaders
            {
                const auto shaders = shader_files();
                const auto shader_dir = project_dir / "shaders";
                const auto shader_out = "shaders/" + (shaders + ".spv");

                cmd += " && ";
                cmd += "glslangValidator -V ";
                cmd += "shaders/" + shaders + " -o "; // shader in cmd
                cmd += shader_out;                    // shader out cmd
            }
            break;
        };
        case BuildPanel::Flags::R: {
            cmd = build;
            break;
        };
        }
    }

    return execute(cmd);
};

// TODO:add progress bar when executing this function
std::string BuildPanel::execute(const std::string &cmd) const {
    std::string result;
    // run command
    {
        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            return result =
                       "Failed to run the command. Error occured somewhere";
        }

        // temporary buffer to read chunks of result
        char buffer[128];

        // append buffer to result
        while (fgets(buffer, sizeof(buffer), pipe)) {
            result += buffer;
        }

        pclose(pipe);
    }

    return result;
};
