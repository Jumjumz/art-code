#pragma once

#include "artboard_sizes.hpp"
#include "start_screen.hpp"

class TemplateGallery : public StartScreen {
  public:
    TemplateGallery();

    void render() override;

  private:
    void set_artboard_template(const TemplateSizes &temp) override;

    glm::vec3 get_artboard_size() const override;

    bool dimensions_acquired() const override;

    TemplateSizes templates;
};
