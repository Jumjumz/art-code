#include "ui_manager.hpp"

UIManager::UIManager() {};

void UIManager::render() {
    // TODO:: add a welcome UI like PS that displays the type size of artboard a user can do

    // render main navigation panel
    this->main_nav.render();

    // TODO: add condition to render workspace or start menu

    for (const auto &workspace : this->workspace_ui) {
        workspace->render();
    }
};
