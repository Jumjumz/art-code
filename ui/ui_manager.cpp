#include "ui_manager.hpp"
#include "start_screen/artboard_settings.hpp"
#include "start_screen/template_gallery.hpp"
#include "workspace/canvas.hpp"
#include "workspace/development.hpp"

#include <memory>

UIManager::UIManager() {
    this->artboard_size = {0.0f, 0.0f, 0.0f};
    // init start screen
    this->start_ui.push_back(std::make_unique<TemplateGallery>());
    this->start_ui.push_back(std::make_unique<ArtboardSettings>());

    // init workspace
    this->workspace_ui.push_back(std::make_unique<Development>());
    this->workspace_ui.push_back(std::make_unique<Canvas>());
};

void UIManager::render() {
    // render main navigation panel
    this->main_nav.render();

    if (!this->show_main_ui) {
        for (const auto &start_screen : this->start_ui) {
            // immidiately render the start screen ui's
            start_screen->render();

            // check if artboard dimensions is now ready
            if (start_screen->dimensions_acquired()) {
                this->artboard_size = start_screen->get_artboard_size();
                this->show_main_ui = true;

                // clear the vector and its shared ptr
                this->start_ui.clear();
                this->start_ui.shrink_to_fit();
                break;
            }
        }
    } else {
        for (const auto &workspace : this->workspace_ui) {
            workspace->render();
        }
    }
};
