#include "build_panel.hpp"
#include "imgui.h"
#include "nav_items.hpp"

BuildPanel::BuildPanel() {

};

void BuildPanel::render() {
    auto panel_size = ImGui::GetContentRegionAvail();
    float width = 100.0f;
    float height = 20.0f;

    for (const auto &[action, shortcut] : NavBuildItems::PANEL) {
        ImGui::SetCursorPosY((panel_size.y - height) / 2.0f);
        ImGui::Button(action.c_str(), ImVec2{width, height});
        ImGui::SameLine();
    }
};
