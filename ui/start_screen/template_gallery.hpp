#pragma once

#include "artboard_sizes.hpp"
#include "start_screen.hpp"

class TemplateGallery : public StartScreen {
  public:
    TemplateGallery();

    void render() override;

  private:
    void set_artboard_dimensions(const glm::vec3 &dimensions) override;

    glm::vec3 get_artboard_size() const override;

    bool dimensions_acquired() const override;

    void create_new_project(const glm::vec3 &dimensions) override;

    glm::vec3 set_artboard_template(const TemplateSizes &temp);
};
