#pragma once

#include "ui.hpp"
#include "workspace/canvas.hpp"
#include "workspace/development.hpp"
#include "workspace/navigation.hpp"
#include <memory>
#include <vector>

class UIManager {
  public:
    UIManager();

    void render() const;

  private:
    // TODO: add vector for start screen components
    const std::vector<std::shared_ptr<WorkSpace>> main_ui = {
        std::make_shared<Navigation>(),
        std::make_shared<Development>(),
        std::make_shared<Canvas>(),
    };

    bool show_main_ui = false;
};
