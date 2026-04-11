#include "build_panel.hpp"
#include "nav_items.hpp"

BuildPanel::BuildPanel() {
    ImGui::FileBrowser file_browser(ImGuiFileBrowserFlags_CloseOnEsc |
                                    ImGuiFileBrowserFlags_CreateNewDir |
                                    ImGuiFileBrowserFlags_MultipleSelection);
    this->file_explorer = file_browser;

    this->file_explorer.SetTitle("Project Folder");
};

void BuildPanel::render() {
    const auto panel_size = ImGui::GetContentRegionAvail();
    const float width = 100.0f;
    const float height = 20.0f;

    for (const auto &[action, shortcut] : NavBuildItems::PANEL) {
        ImGui::SetCursorPosY((panel_size.y - height) / 2.0f);
        if (ImGui::Button(action.c_str(), ImVec2{width, height})) {
            if (action == "Files") {
                this->file_explorer.SetDirectory(ProjectPath::get_project_path());
                file_explorer.Open();
            }
        }
        ImGui::SameLine();
    }

    this->file_explorer.Display();

    if (this->file_explorer.HasSelected()) {
        this->file_explorer.ClearSelected();
    }
};
