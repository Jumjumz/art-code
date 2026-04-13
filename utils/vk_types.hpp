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

// TODO: create a canvas class specifically design for its own resources to avoid using global variables
struct CanvasUtils {
    // zoom variable
    static inline float zoom = 1;
    // panning
    static inline glm::vec2 panning = {0.0f, 0.0f};
    // most last position
    static inline glm::vec2 mouse_last_pos = {0.0f, 0.0f};
    // use in art code and canvas class
    static inline VkDescriptorSet canvas_texture = VK_NULL_HANDLE;
};

struct TextEditorUtils {
    static inline bool file_save = false;
};
