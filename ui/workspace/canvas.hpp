#pragma once

#include "work_space.hpp"

class Canvas : public WorkSpace {
  public:
    Canvas();

    void render() override;

  private:
};
