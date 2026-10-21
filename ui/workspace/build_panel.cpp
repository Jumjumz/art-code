#include "build_panel.hpp"
#include "imgui.h"
#include "json.hpp"
#include "nav_items.hpp"
#include "vk_types.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

void BuildPanel::render() {
    const auto& panel_size = ImGui::GetContentRegionAvail();
    const float width = 100.0f, height = 20.0f;

    bool show_adding_includes = false;

    for (const auto& [action, shortcut] : NavBuildItems::PANEL) {
        ImGui::SetCursorPosY((panel_size.y - height) / 2.0f);
        if (ImGui::Button(action.c_str(), ImVec2{width, height})) {
            if (action == "Includes") {
                // auto add .cpp files in proj dir to solution file
                show_adding_includes = true;
                add_includes();
            }

            if (action == "Build")
                compile();

            if (action == "Run") {
                if (!this->project_compiled)
                    compile();

                // 0 = compilation success
                if (ExecuteResult::get_exit_code() == 0) {
                    create_cmd(BuildPanel::Flags::R);
                    this->project_compiled = false;
                }
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
    // run compile command
    const auto& cmd = create_cmd(BuildPanel::Flags::C);
    execute(cmd);
    this->project_compiled = true;
};

void BuildPanel::add_includes() const {
    const auto& solution_file = ProjectPath::get_solution_file();

    nlohmann::json js;
    {
        std::ifstream read(solution_file);
        js = nlohmann::json::parse(read);
    }

    auto includes = nlohmann::json::array();
    // search for new .cpp files in project dir
    for (const auto& file : fs::recursive_directory_iterator(solution_file.parent_path())) {
        const auto file_path = file.path();
        if (file_path.extension() == ".cpp") {
            const auto relative_path = fs::relative(file_path, solution_file.parent_path());
            // exclude known files
            if (relative_path == "components/comp.cpp" || relative_path == "main.cpp") {
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

std::string BuildPanel::executable_files() const {
    // read solution file
    std::vector<std::string> executables = {};
    {
        const auto&   solution_file = ProjectPath::get_solution_file();
        std::ifstream read(solution_file);

        nlohmann::json js;
        js = nlohmann::json::parse(read);
        read.close();

        const auto includes = js["includes"].get<std::vector<std::string>>();
        executables         = js["sources"].get<std::vector<std::string>>();
        // append includes if not empty
        if (!includes.empty()) {
            executables.insert(executables.end(), includes.begin(), includes.end());
        }
    }

    std::string source = "";
    for (const auto& dir : executables) {
        source += dir + " "; // add space at the end of each path
    }

    return source;
};

std::string BuildPanel::create_cmd(const BuildPanel::Flags& flag) {
    std::string cmd;
    // execute and use gcc compiler
    {
        const auto&           project_dir = ProjectPath::get_project_path();
        std::filesystem::path build       = "";
#ifdef _WIN32
        build = project_dir / "build";
#else
        build = project_dir / "build" / "artcode";
#endif

        // TODO:might need to move the shader compilation to compile
        switch (flag) {
        case BuildPanel::Flags::C: {
            const auto executables = executable_files();
            // NOTE:in the future support multiple compilers
            // compile user codes
#ifdef _WIN32
            {
                char exe_path[MAX_PATH];
                GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
                fs::path exe_dir = fs::path(exe_path).parent_path();
                fs::path api_dir = exe_dir / "api";

                cmd  = "cd " + project_dir.string();
                cmd += " && ";
                // TODO:make finding the vcvars64.bat dynamic
                cmd += "\"C:\\Program Files\\Microsoft Visual "
                       "Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"";
                cmd += " && ";
                // MSVC compiler
                cmd += "cl /EHsc /std:c++20 /MT /GS- " + executables;
                cmd += "/I\"" + api_dir.string() + "\" "; // include dir

                // output location
                cmd += "/Fo:\"" + build.string() + "\\\\\" "; // obj to build dir
                cmd += "/link ";
                cmd += "\"" + (api_dir / "api.lib").string() + "\" ";        // link .lib
                cmd += "/OUT:\"" + (build / "artcode.exe").string() + "\" "; // exe output
                cmd += " 2>&1";

                // Copy api.dll
                fs::copy_file(api_dir / "api.dll", build / "api.dll",
                              fs::copy_options::overwrite_existing);
            }
#else
            {
                // change to project dir before compiling
                cmd  = "cd " + project_dir.string();
                cmd += " && ";
                // access the api dir to locate artcode.hpp library
                fs::path exe_dir = fs::canonical("/proc/self/exe").parent_path();
                fs::path api_dir = exe_dir / "api";

                // compile using gcc
                cmd += "g++ -std=c++20 " + executables;
                // compile main with artcode shared lib
                cmd += "-I" + api_dir.string() + " ";
                cmd += "-L" + api_dir.string() + " ";
                cmd += "-lapi ";
                cmd += "-Wl,-rpath," + api_dir.string() + " ";
                cmd += "-o " + build.string() + " 2>&1";
            }
#endif
            break;
        };
        case BuildPanel::Flags::R: {
            int exit = 0;
#ifdef _WIN32
            cmd  = (build / "artcode.exe").string();
            exit = execute(cmd);
#else
            cmd  = build.string();
            exit = execute(cmd);
#endif
            // stop execution, result will display runtime errors
            if (exit != 0)
                break;

            ShadersCompiled::is_compiled = true;
            break;
        };
        }
    }

    return cmd;
};

// TODO:add progress bar/indicator when executing this function
// add windows implementation
int BuildPanel::execute(const std::string& cmd) {
    std::string result    = "";
    FILE*       pipe      = nullptr;
    int         exit_code = 0;

#ifdef _WIN32
    pipe = _popen(cmd.c_str(), "r");
#else
    pipe = popen(cmd.c_str(), "r");
#endif
    if (pipe == nullptr) {
        int return_err_code = -1;
        result              = "Failed to run the command. Error occured somewhere";
        ExecuteResult::set_result(result);
        ExecuteResult::set_exit_code(return_err_code);
        return return_err_code;
    }

    // temporary buffer to read chunks of result
    char buffer[128];

    // append buffer to result
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
#ifdef _WIN32
    exit_code = _pclose(pipe);
#else
    exit_code = pclose(pipe);
#endif
    // set global variables
    ExecuteResult::set_exit_code(exit_code);
    ExecuteResult::set_result(result);

    return exit_code;
};
