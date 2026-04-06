#pragma once

#include "start_screen.hpp"

class ArtboardSettings : public StartScreen {
  public:
    ArtboardSettings();

    void render() override;

  private:
    void set_artboard_custom(const glm::vec3 &dimensions) override;

    glm::vec3 get_artboard_size() const override;

    bool dimensions_acquired() const override;
};
