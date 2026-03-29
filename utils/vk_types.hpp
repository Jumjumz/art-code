#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// artboard
struct ArtboardBuffer {
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
    glm::vec2 reso;
};

struct CanvasUtils {
    // use in art code and canvas class
    inline static VkDescriptorSet canvas_texture = VK_NULL_HANDLE;
    // zoom variable
    inline static float zoom = 1;
};
