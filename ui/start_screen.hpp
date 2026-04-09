#pragma once

#include "artboard_sizes.hpp"
#include "build.hpp"
#include "imgui.h"
#include <cstdlib>
#include <glm/glm.hpp>
#include <imfilebrowser.h>

class StartScreen {
  public:
    virtual ~StartScreen() = default;

    virtual void render() = 0;

    virtual glm::vec3 get_artboard_size() const { return this->artboard_size; };

    virtual bool dimensions_acquired() const { return this->has_dimensions; };

  protected:
    ImGuiViewport *viewport = nullptr;
    ImVec2 work_size = ImVec2{0.0f, 0.0f};
    ImVec2 work_pos = ImVec2{0.0f, 0.0f};

    glm::vec3 artboard_size = {0.0f, 0.0f, 0.0f};
    bool has_dimensions = false;

    Build build;

    const char *home = getenv("HOME");

    ImGui::FileBrowser file_dialog;

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
