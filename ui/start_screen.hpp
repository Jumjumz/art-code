#pragma once

#include "build.hpp"
#include "imgui.h"
#include "json.hpp"
#include "nav_items.hpp"
#include <cstdlib>
#include <fstream>
#include <glm/glm.hpp>
#include <imfilebrowser.h>
#include <iostream>

class StartScreen {
  public:
    virtual ~StartScreen() = default;

    virtual void render() = 0;

    virtual glm::vec3 get_artboard_size() const { return this->artboard_size; };

    virtual bool dimensions_acquired() const { return this->has_dimensions; };

  protected:
    ImGuiViewport *viewport = nullptr;
    ImVec2 work_size = ImVec2{0.0f, 0.0f};
    ImVec2 work_pos = ImVec2{0.0f, 0.0f};

    glm::vec3 artboard_size = {0.0f, 0.0f, 0.0f};
    bool has_dimensions = false;

    bool open_selected = false; // for open button

    Build build;

    const char *home = getenv("HOME"); // set home directory

    ImGui::FileBrowser file_dialog;

    virtual void set_artboard_dimensions(const glm::vec3 &dimensions) {
        this->artboard_size = dimensions;
        this->has_dimensions = false;
    };

    virtual void create_new_project(const glm::vec3 &dimensions) {
        this->artboard_size = dimensions;
        this->has_dimensions = false;
    };

    // implementation is the same for both derived classes
    virtual void get_artboard_solution() {
        if (this->file_dialog.HasSelected()) {
            nlohmann::json js;
            const auto solution = this->file_dialog.GetSelected();

            if (solution.extension() == this->build.sln_ext) {
                // read and parse solution file
                std::ifstream read(solution);
                js = nlohmann::json::parse(read);
                auto artboard_size = js["artboard_size"];

                // set the project path for text editor
                ProjectPath::set_project_path(js["project_path"]);

                // set solution file
                ProjectPath::set_solution_file(solution);

                // get width and height
                const auto dimensions =
                    glm::vec3{artboard_size["width"], artboard_size["height"],
                              artboard_size["ppi"]};

                read.close();

                set_artboard_dimensions(dimensions);
            } else {
                std::cerr << "File not readable for this program" << std::endl;
            }
            this->file_dialog.ClearSelected();
        }
    };

  private:
};
