#pragma once

#include "main_nav.hpp"
#include "start_screen.hpp"
#include "start_screen/artboard_settings.hpp"
#include "start_screen/template_gallery.hpp"
#include "work_space.hpp"
#include "workspace/canvas.hpp"
#include "workspace/development.hpp"
#include <memory>
#include <vector>

class UIManager {
  public:
    UIManager();

    void render();

    glm::vec3 artboard_size;

  private:
    const MainNavigation main_nav;

    std::vector<std::shared_ptr<StartScreen>> start_ui = {
        std::make_shared<TemplateGallery>(),
        std::make_shared<ArtboardSettings>()};

    const std::vector<std::shared_ptr<WorkSpace>> workspace_ui = {
        std::make_shared<Development>(),
        std::make_shared<Canvas>(),
    };

    bool show_main_ui = false;
};
