#include "build_panel.hpp"
#include "imgui.h"
#include "nav_items.hpp"

#include <cstdio>
#include <filesystem>
#include <iostream>

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
                std::cerr << "pressed!" << std::endl;
            }

            if (action == "Run") {
                const auto result = execute(Flags::R);
                CompilerResult::set_run_result(result);
            }
        }
        ImGui::SameLine();
    }
};

// TODO: dont hardcode the files and possibly use dynamic adding of .cpp files for linking
std::string BuildPanel::execute(const Flags &flag) {
    std::string cmd;
    std::string result;
    const auto project_dir = ProjectPath::get_project_path();

    if (flag == Flags::C) {
        const std::string source = project_dir / "main.cpp";
        const std::string build = project_dir / "build/artcode";
        cmd = "g++ -std=c++20 " + source + " -o " + build + " 2>&1";
    } else if (flag == Flags::R) {
        cmd = project_dir.string() + "/build/artcode";
    }

    // execute and use gcc compiler
    {
        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe)
            std::cerr << "Failed to run the command" << std::endl;

        // temporary buffer to read chunks of result
        char buffer[128];

        while (fgets(buffer, sizeof(buffer), pipe)) {
            result += buffer;
        }

        pclose(pipe);
    }

    return result;
};
