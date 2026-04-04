#pragma once

#include "artboard_sizes.hpp"
#include "ui.hpp"
#include <glm/fwd.hpp>

class TemplateGallery : public StartScreen {
  public:
    TemplateGallery();

    void render() override;

  private:
    void artboard(const TemplateSizes &temp);
};
