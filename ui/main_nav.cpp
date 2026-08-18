#include "main_nav.hpp"
#include "nav_items.hpp"

void MainNavigation::render() {
    if (ImGui::BeginMainMenuBar()) {
        for (const auto& [menus, items] : NavMainItems::MENUS) {
            if (ImGui::BeginMenu(menus.c_str())) {
                for (const auto& [item_label, item_shortcut] : items) {
                    if (ImGui::MenuItem(item_label.c_str(), item_shortcut.c_str())) {
                        if (item_label == "Save") {
                            if (SaveFile::get_save_path().empty()) {
                                this->file_dialog = ImGui::FileBrowser{
                                    ImGuiFileBrowserFlags_EnterNewFilename, this->home_dir};
                            } else {
                                // get the latest save path for dialog
                                this->file_dialog = ImGui::FileBrowser{
                                    ImGuiFileBrowserFlags_EnterNewFilename,
                                    SaveFile::get_save_path().parent_path()};
                            }
                            // sets init name, can be renamed
                            this->file_dialog.SetInputName("image");
                            this->file_dialog.SetTitle("Save art as png");
                            this->file_dialog.Open();
                            break;
                        }
                        // NOTE:commented to do png for now
                        /*else if (item_label == "Save as") {
                                      this->file_dialog.SetTitle("Save art as");
                                      this->file_dialog.Open();
                                      break;
                                  }*/
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
    this->file_dialog.Display();

    // pass the selected dir for saving
    if (this->file_dialog.HasSelected()) {
        SaveFile::set_save_path(this->file_dialog.GetSelected());
        SaveFile::has_path = true;

        this->file_dialog.ClearSelected();
        this->file_dialog.Close();
    }
}
