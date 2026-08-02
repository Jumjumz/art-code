#include "main_nav.hpp"
#include "imgui.h"
#include "nav_items.hpp"
#include <iostream>

// TODO:create a util file for home directory and other related things
MainNavigation::MainNavigation() {
    ImGui::FileBrowser file(ImGuiFileBrowserFlags_SelectDirectory |
                                ImGuiFileBrowserFlags_CreateNewDir,
                            getenv("HOME"));

    this->file_dialog = file;
};

void MainNavigation::render() {
    if (ImGui::BeginMainMenuBar()) {
        for (const auto& [menus, items] : NavMainItems::MENUS) {
            if (ImGui::BeginMenu(menus.c_str())) {
                for (const auto& [item_label, item_shortcut] : items) {
                    // TODO:create global variable for path
                    if (ImGui::MenuItem(item_label.c_str(), item_shortcut.c_str())) {
                        if (item_label == "Save") {
                            this->file_dialog.SetTitle("Save art");
                            this->file_dialog.Open();
                            break;
                        } else if (item_label == "Save as") {
                            this->file_dialog.SetTitle("Save art as");
                            this->file_dialog.Open();
                            break;
                        }
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }

    this->file_dialog.Display();

    if (this->file_dialog.HasSelected()) {
        std::cout << this->file_dialog.GetSelected() << std::endl;
    }
};
