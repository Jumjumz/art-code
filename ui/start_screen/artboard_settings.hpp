#pragma once

#include "start_screen.hpp"

class ArtboardSettings : public StartScreen {
  public:
    ArtboardSettings();

    void render() override;

  private:
    bool open_selected = false;

    void set_artboard_dimensions(const glm::vec3 &dimensions) override;

    glm::vec3 get_artboard_size() const override;

    bool dimensions_acquired() const override;

    void get_artboard_solution() override;
};
