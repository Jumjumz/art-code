#pragma once

#include "main_nav.hpp"
#include "start_screen.hpp"
#include "work_space.hpp"

#include <memory>
#include <vector>

class UIManager {
  public:
    UIManager();

    void render();

    glm::vec3 artboard_size;

  private:
    const MainNavigation main_nav;

    std::vector<std::unique_ptr<StartScreen>> start_ui = {};

    std::vector<std::unique_ptr<WorkSpace>> workspace_ui = {};

    bool show_main_ui = false;
};
