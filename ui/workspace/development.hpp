#pragma once

#include "build_panel.hpp"
#include "console.hpp"
#include "text_editor.hpp"
#include "work_space.hpp"

class Development : public WorkSpace {
  public:
    Development();

    void render() override;

  private:
    BuildPanel build_panel;
    TextEditorWrapper text_editor;
    Console console;
};
