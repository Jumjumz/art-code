#include "build_panel.hpp"
#include "imgui.h"
#include "json.hpp"
#include "nav_items.hpp"
#include <filesystem>
#include <fstream>
#include <unistd.h>

BuildPanel::BuildPanel() {};

void BuildPanel::render() {
    const auto panel_size = ImGui::GetContentRegionAvail();
    const float width = 100.0f;
    const float height = 20.0f;

    for (const auto &[action, shortcut] : NavBuildItems::PANEL) {
        ImGui::SetCursorPosY((panel_size.y - height) / 2.0f);
        if (ImGui::Button(action.c_str(), ImVec2{width, height})) {
            if (action == "Build") {
                const auto result = execute(Flags::C);
                CompilerResult::set_compiler_result(result);
                // prevent persistent data of run result when recompiling
                CompilerResult::set_run_result("");
            }

            if (action == "Run") {
                const auto result = execute(Flags::R);
                CompilerResult::set_run_result(result);
            }
        }
        ImGui::SameLine();
    }
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

std::string BuildPanel::execute(const Flags &flag) const {
    std::string cmd;
    // execute and use gcc compiler
    {
        const auto project_dir = ProjectPath::get_project_path();
        const std::string build = project_dir / "build/artcode";
        if (flag == Flags::C) {
            const auto executables = executable_files();
            // chage dir to project dir before compiling
            std::filesystem::path exe_dir =
                std::filesystem::canonical("/proc/self/exe").parent_path();
            std::filesystem::path api_dir = exe_dir / "api";

            // compile main with artcode shared lib
            cmd = "cd " + project_dir.string() + " && " + "g++ -std=c++20 " +
                  executables;
            cmd += "-I" + api_dir.string() + " ";
            cmd += "-L" + api_dir.string() + " ";
            cmd += "-lapi ";
            cmd += "-Wl,-rpath," + api_dir.string() + " ";
            cmd += "-o " + build + " 2>&1";
        } else if (flag == Flags::R) {
            cmd = build;
        }
    }

    std::string result;
    // store compiler result
    {
        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            return result =
                       "Failed to run the command. Error occured somewhere";
        }

        // temporary buffer to read chunks of result
        char buffer[128];

        while (fgets(buffer, sizeof(buffer), pipe)) {
            result += buffer;
        }

        pclose(pipe);
    }

    return result;
};
