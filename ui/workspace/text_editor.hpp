#pragma once

#include "TextEditor.h"
#include "ui.hpp"

class TextEditorWrapper : public WorkSpace {
  public:
    TextEditorWrapper();

    void render() override;

  private:
    TextEditor editor;
};
