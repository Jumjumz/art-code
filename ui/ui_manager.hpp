#pragma once

#include "main_nav.hpp"
#include "ui.hpp"
#include "workspace/canvas.hpp"
#include "workspace/development.hpp"
#include <memory>
#include <vector>

class UIManager {
  public:
    UIManager();

    void render();

  private:
    MainNavigation main_nav;
    // TODO: add vector for start screen components
    const std::vector<std::shared_ptr<WorkSpace>> workspace_ui = {
        std::make_shared<Development>(),
        std::make_shared<Canvas>(),
    };

    bool show_main_ui = false;
};
