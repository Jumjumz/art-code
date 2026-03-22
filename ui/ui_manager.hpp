#pragma once

#include "ui.hpp"
#include "workspace/navigation.hpp"
#include "workspace/text_editor.hpp"
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
        std::make_shared<TextEditor>(),
    };

    bool show_main_ui = false;
};
