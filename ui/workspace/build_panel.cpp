#include "build_panel.hpp"
#include "imgui.h"
#include "nav_items.hpp"

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

// TODO: add a way to write in solution file for the executable files
std::string BuildPanel::executable_files(const std::filesystem::path &project_dir) {
    const std::vector<std::string> dirs = {project_dir / "main.cpp",
                                           project_dir / "components/comp.cpp"};
    std::string source;
    for (const auto &dir : dirs) {
        source += dir + " "; // add space at the end of each path
    }

    return source;
};

// TODO: dont hardcode the files and possibly use dynamic adding of .cpp files for linking
// TODO: might actually need to create a library for dynamic adding of .cpp files
std::string BuildPanel::execute(const Flags &flag) {
    std::string cmd;
    {
        const auto project_dir = ProjectPath::get_project_path();
        const std::string build = project_dir / "build/artcode";
        if (flag == Flags::C) {
            const auto executables = executable_files(project_dir);
            cmd = "g++ -std=c++20 " + executables + "-o " + build + " 2>&1";
        } else if (flag == Flags::R) {
            cmd = build;
        }
    }

    std::string result;
    // execute and use gcc compiler
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
