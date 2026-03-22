#include "navigation.hpp"
#include "imgui.h"
#include "nav_items.hpp"

Navigation::Navigation() {};

void Navigation::render() {
    if (ImGui::BeginMainMenuBar()) {
        for (const auto &[menus, items] : NavMainItems::MENUS) {
            if (ImGui::BeginMenu(menus.c_str())) {
                for (const auto &[item_label, item_shortcut] : items) {
                    ImGui::MenuItem(item_label.c_str(), item_shortcut.c_str());
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
};
