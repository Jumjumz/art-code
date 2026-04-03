#pragma once

#include "artboard_sizes.hpp"
#include "ui.hpp"

class TemplateGallery : public StartScreen {
  public:
    TemplateGallery();

    void render() override;

  private:
    void web_template_size(const TemplateSizes &temp);
};
