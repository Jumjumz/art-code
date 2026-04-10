#include "build_panel.hpp"
#include "nav_items.hpp"

BuildPanel::BuildPanel() {};

void BuildPanel::render() {
    const auto panel_size = ImGui::GetContentRegionAvail();
    const float width = 100.0f;
    const float height = 20.0f;

    ImGui::FileBrowser file_explorer(ImGuiFileBrowserFlags_CloseOnEsc |
                                     ImGuiFileBrowserFlags_CreateNewDir |
                                     ImGuiFileBrowserFlags_MultipleSelection);

    if (!this->build.get_project_directory().empty()) {
        file_explorer.SetTitle("Project Folder");
    }

    for (const auto &[action, shortcut] : NavBuildItems::PANEL) {
        ImGui::SetCursorPosY((panel_size.y - height) / 2.0f);
        if (ImGui::Button(action.c_str(), ImVec2{width, height})) {
            if (action == "Files") {
                file_explorer.Open();
            }
        }
        ImGui::SameLine();
    }

    file_explorer.Display();

    if (file_explorer.HasSelected()) {
        file_explorer.ClearSelected();
    }
};
