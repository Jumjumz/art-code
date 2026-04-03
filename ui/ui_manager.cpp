#include "ui_manager.hpp"
#include "artboard_sizes.hpp"

UIManager::UIManager() {};

void UIManager::render() {
    // render main navigation panel
    this->main_nav.render();

    if (ArtboardSize::ppi != 0)
        this->show_main_ui = true;

    if (!this->show_main_ui) {
        for (const auto &start_screen : this->start_ui) {
            start_screen->render();
        }
    } else {
        for (const auto &workspace : this->workspace_ui) {
            workspace->render();
        }
    }
};
