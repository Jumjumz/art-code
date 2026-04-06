#pragma once

#include "artboard_sizes.hpp"
#include "build.hpp"
#include <glm/glm.hpp>

class StartScreen {
  public:
    virtual ~StartScreen() = default;

    virtual void render() = 0;

    virtual glm::vec3 get_artboard_size() const { return this->artboard_size; };

    virtual bool dimensions_acquired() const { return this->has_dimensions; };

  protected:
    Build build;

    glm::vec3 artboard_size = {0.0f, 0.0f, 0.0f};
    bool has_dimensions = false;

    virtual void set_artboard_template(const TemplateSizes &temp) {
        switch (temp) {
        default: {
            this->artboard_size = {0.0f, 0.0f, 0.0f};
            this->has_dimensions = false;
        };
        }
    };
    virtual void set_artboard_custom(const glm::vec3 &dimensions) {
        this->artboard_size = dimensions;
        this->has_dimensions = false;
    };

  private:
};

class WorkSpace {
  public:
    virtual ~WorkSpace() = default;

    virtual void render() = 0;

  protected:
  private:
};
