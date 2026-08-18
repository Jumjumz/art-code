#include "ui_manager.hpp"
#include "start_screen/artboard_settings.hpp"
#include "start_screen/template_gallery.hpp"
#include "vk_types.hpp"
#include "workspace/canvas.hpp"
#include "workspace/development.hpp"

UIManager::UIManager()
    : artboard_size(100.0f, 100.0f, 72.0f) {
    // init start screen
    this->start_ui.push_back(std::make_unique<TemplateGallery>());
    this->start_ui.push_back(std::make_unique<ArtboardSettings>());
};

void UIManager::render() {
    // render main navigation panel
    this->main_nav.render();

    // main ui is the workspace
    if (!this->show_main_ui) {
        for (const auto& start_screen : this->start_ui) {
            // immidiately render the start screen ui's
            start_screen->render();

            // check if artboard dimensions is ready
            if (start_screen->dimensions_acquired()) {
                this->artboard_size = start_screen->get_artboard_size();
                // uses global variable to render texture in canvas
                Artboard::set_artboard_size({this->artboard_size.x, this->artboard_size.y});

                // clear the vector and its uniq ptr
                this->start_ui.clear();
                this->start_ui.shrink_to_fit();

                // it only needs to init if start ui is done rendering and data is ready
                this->workspace_ui.push_back(std::make_unique<Canvas>());
                this->workspace_ui.push_back(std::make_unique<Development>());

                this->show_main_ui = true;
                break;
            }
        }
    } else {
        for (const auto& workspace : this->workspace_ui) {
            workspace->render();
        }
    }
};
