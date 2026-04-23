#pragma once

#include "start_screen.hpp"

class ArtboardSettings : public StartScreen {
  public:
    ArtboardSettings();

    void render() override;

  private:
    void set_artboard_dimensions(const glm::vec3 &dimensions) override;

    void create_new_project(const glm::vec3 &dimensions) override;
};
