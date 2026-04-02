#pragma once

#include "TextEditor.h"

class TextEditorWrapper {
  public:
    TextEditorWrapper();

    void render();

  private:
    TextEditor editor;
};
