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
    // panning
    inline static glm::vec2 panning = {0.0f, 0.0f};
    // most last position
    inline static glm::vec2 mouse_last_pos = {0.0f, 0.0f};
};
