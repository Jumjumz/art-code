#include "build_panel.hpp"
#include "imgui.h"
#include "nav_items.hpp"

BuildPanel::BuildPanel() {

};

void BuildPanel::render() {
    for (const auto &[action, shortcut] : NavBuildItems::PANEL) {
        ImGui::Button(action.c_str());
        ImGui::SameLine();
    }
};
