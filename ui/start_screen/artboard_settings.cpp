#include "artboard_settings.hpp"
#include "nav_items.hpp"

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

    // display file modal browser
    this->file_dialog.Display();

    if (!this->open_selected) {
        create_new_project(glm::vec3{ab_width, ab_height, 72.0f});
    } else {
        get_artboard_solution();
    }
};

void ArtboardSettings::create_new_project(const glm::vec3 &dimensions) {
    if (this->file_dialog.HasSelected()) {
        if (this->build.set_project_directory(this->file_dialog.GetSelected(),
                                              dimensions)) {
            // pass path to global project path
            ProjectPath::set_project_path(this->file_dialog.GetSelected());
            this->file_dialog.ClearSelected();

            set_artboard_dimensions(dimensions);
            get_solution_file(ProjectPath::get_project_path());
        } else {
            this->file_dialog.ClearSelected();
            this->file_dialog.Close();
        }
    }
};

void ArtboardSettings::set_artboard_dimensions(const glm::vec3 &dimensions) {
    this->artboard_size = dimensions;
    this->has_dimensions = true;
};
