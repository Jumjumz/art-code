#include "navigation.hpp"
#include "imgui.h"
#include "nav_items.hpp"

Navigation::Navigation() {};

void Navigation::render() {
    if (ImGui::BeginMainMenuBar()) {
        for (const auto &menus : NavMainItems::MENUS) {
            if (ImGui::BeginMenu(menus.first.c_str())) {
                for (const auto &items : menus.second) {
                    ImGui::MenuItem(items.first.c_str(), items.second.c_str());
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
};
