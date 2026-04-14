#include "artboard_settings.hpp"
#include "json.hpp"
#include "nav_items.hpp"
#include <fstream>
#include <iostream>

ArtboardSettings::ArtboardSettings() {
    ImGui::FileBrowser file(ImGuiFileBrowserFlags_CreateNewDir |
                                ImGuiFileBrowserFlags_SelectDirectory,
                            this->home);

    this->file_dialog = file;
    this->file_dialog.SetTitle("Create Custom Project");
};

void ArtboardSettings::render() {
    this->viewport = ImGui::GetMainViewport();
    this->work_size = this->viewport->WorkSize;
    this->work_pos = this->viewport->WorkPos;

    const float width = this->work_size.x * 0.3f;
    const float height = this->work_size.y;
    const float pos_x = this->work_size.x - width;
    const float pos_y = this->work_pos.y;

    ImGui::SetNextWindowSize(ImVec2{width, height});
    ImGui::SetNextWindowPos(ImVec2{pos_x, pos_y});

    // init values
    static float ab_width = 1920.0f;
    static float ab_height = 1080.0f;

    const std::vector<std::string> buttons = {"Create", "Open"};

    // removes padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{20.0f, 20.0f});

    ImGui::Begin("##artboard-settings", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Artboard Settings");
    ImGui::DragFloat("Width", &ab_width, 1.0f, 10.0f, 5000.0f, "%.0f");
    ImGui::DragFloat("Height", &ab_height, 1.0f, 10.0f, 5000.0f, "%.0f");

    for (const auto &button : buttons) {
        if (ImGui::Button(button.c_str(), ImVec2{60, 20})) {
            // TODO: clean up
            if (button == "Create") {
                // reinitialize file browser
                ImGui::FileBrowser file(ImGuiFileBrowserFlags_CreateNewDir |
                                            ImGuiFileBrowserFlags_SelectDirectory,
                                        this->home);

                this->file_dialog = file;
                this->file_dialog.SetTitle("Create Custom Project");

                this->file_dialog.Open();

                this->open_selected = false;
            }

            if (button == "Open") {
                // reset file_dialog for button open
                ImGui::FileBrowser file(ImGuiFileBrowserFlags_CloseOnEsc |
                                            ImGuiFileBrowserFlags_MultipleSelection,
                                        this->home);

                this->file_dialog = file;
                this->file_dialog.SetTitle("Open Custom Project");
                this->file_dialog.Open();

                this->open_selected = true;
            }
        }
        ImGui::SameLine();
    }
    ImGui::End();
    ImGui::PopStyleVar();

    // TODO: assign a virtual function for the creating project
    if (!this->open_selected) {
        // display file modal browser
        this->file_dialog.Display();

        if (this->file_dialog.HasSelected()) {
            if (this->build.set_project_directory(
                    this->file_dialog.GetSelected(),
                    glm::vec3{ab_width, ab_height, 72.0f})) {
                // pass the path to global project path
                ProjectPath::set_project_path(this->file_dialog.GetSelected());
                this->file_dialog.ClearSelected();

                set_artboard_dimensions(glm::vec3{ab_width, ab_height, 72.0f});
            } else {
                this->file_dialog.ClearSelected();
                this->file_dialog.Close();
            }
        }
    } else {
        get_artboard_solution();
    }
};

void ArtboardSettings::get_artboard_solution() {
    this->file_dialog.Display();

    if (this->file_dialog.HasSelected()) {
        nlohmann::json js;
        const auto solution = this->file_dialog.GetSelected();

        if (solution.extension() == ".rcd") {
            // read and parse solution file
            std::ifstream read(solution);
            js = nlohmann::json::parse(read);
            auto artboard_size = js["artboard_size"];

            // set the project path for text editor
            ProjectPath::set_project_path(js["project_path"]);
            // get width and height
            this->artboard_size =
                glm::vec3{artboard_size["width"], artboard_size["height"],
                          artboard_size["ppi"]};

            this->has_dimensions = true;

            read.close();
        } else {
            std::cerr << "File not readable for this program" << std::endl;
        }
    }
};

void ArtboardSettings::set_artboard_dimensions(const glm::vec3 &dimensions) {
    this->artboard_size = dimensions;
    this->has_dimensions = true;
};

glm::vec3 ArtboardSettings::get_artboard_size() const {
    return this->artboard_size;
}

bool ArtboardSettings::dimensions_acquired() const {
    return this->has_dimensions;
};
