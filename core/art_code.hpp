#ifndef ART_CODE_HPP
#define ART_CODE_HPP

#pragma once

#include "vulkan_context.hpp"
#include "window.hpp"

class ArtCode {
  public:
    ArtCode();

    void run();

  private:
    uint32_t width = 1440;
    uint32_t aspect = 16 / 9;

    Window window{this->width, this->aspect};

    VulkanContext ctx{this->window.app_window};

    void loop();

    void cleanup() const;
};

#endif // !ART_CODE_HPP
