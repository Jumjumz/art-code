#include "ui_manager.hpp"

UIManager::UIManager() {};

void UIManager::render() const {
    // TODO:: add a welcome UI like PS that displays the type ofr artboard a user can do

    for (const auto &main_ui : this->main_ui) {
        main_ui->render();
    }
};
