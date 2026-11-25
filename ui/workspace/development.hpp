#pragma once

#include "build_panel.hpp"
#include "console.hpp"
#include "work_space.hpp"

class Development : public WorkSpace {
  public:
    void render() override;

  private:
    BuildPanel build_panel;
    Console    console;
};
