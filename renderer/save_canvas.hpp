#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

class SaveCanvas {
  public:
    SaveCanvas();

    glm::vec2 artboard;

    void save_canvas(const vk::raii::DeviceMemory& memory);

  private:
};
