#include "build_panel.hpp"
#include "imgui.h"
#include "json.hpp"
#include "nav_items.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <imfilebrowser.h>
#include <iostream>
#include <unistd.h>

BuildPanel::BuildPanel() {};

void BuildPanel::render() {
    const auto panel_size = ImGui::GetContentRegionAvail();
    const float width = 100.0f;
    const float height = 20.0f;
    static ImGui::FileBrowser includes;

    for (const auto &[action, shortcut] : NavBuildItems::PANEL) {
        ImGui::SetCursorPosY((panel_size.y - height) / 2.0f);
        if (ImGui::Button(action.c_str(), ImVec2{width, height})) {
            if (action == "Includes") {
                // set file browser
                ImGui::FileBrowser file(ImGuiFileBrowserFlags_CloseOnEsc |
                                            ImGuiFileBrowserFlags_MultipleSelection,
                                        ProjectPath::get_project_path());
                includes = file;
                includes.SetTitle("Add Include Files");
                includes.Open();
            }

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

    includes.Display();

    if (includes.HasSelected()) {
        // pass the selected file
        std::vector<std::string> selected;
        for (const auto &select : includes.GetMultiSelected()) {
            selected.push_back(select.filename());
        }

        add_includes(selected);

        includes.ClearSelected();
    }
};

void BuildPanel::add_includes(const std::vector<std::string> &selected) const {
    const auto solution_file = ProjectPath::get_solution_file();

    nlohmann::json js;
    {
        std::ifstream read(solution_file);
        js = nlohmann::json::parse(read);
    }

    const nlohmann::json includes = js["includes"];
    for (const auto &file : selected) {
        if (std::find(includes.begin(), includes.end(), file) == includes.end()) {
            js["includes"].push_back(file);
        } else {
            std::cerr << "File already included: " << file << std::endl;
        }
    }

    std::ofstream write(solution_file);
    write << js.dump(4);
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
            // access the api dir to locate artcode.hpp library
            std::filesystem::path exe_dir =
                std::filesystem::canonical("/proc/self/exe").parent_path();
            std::filesystem::path api_dir = exe_dir / "api";

            // change dir to project dir before compiling
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
