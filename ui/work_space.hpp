#pragma once

#include "imgui.h"

class WorkSpace {
  public:
    virtual ~WorkSpace() = default;

    virtual void render() = 0;

  protected:
    ImGuiViewport *viewport = nullptr;

    ImVec2 work_size = ImVec2{0.0f, 0.0f};
    ImVec2 work_pos = ImVec2{0.0f, 0.0f};

  private:
};
