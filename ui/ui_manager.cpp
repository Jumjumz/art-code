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

                // clear the vector and its shared ptr
                this->start_ui.clear();
                this->start_ui.shrink_to_fit();

                // this is here because start ui init everything workspace needed
                // it only needs to init if start ui is done rendering and data is prepared
                this->workspace_ui.push_back(std::make_unique<Development>());
                this->workspace_ui.push_back(std::make_unique<Canvas>());

                this->show_main_ui = true;
                break;
            }
        }
    } else {
        for (const auto &workspace : this->workspace_ui) {
            workspace->render();
        }
    }
};
