#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// artboard
struct ArtboardBuffer {
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
    glm::vec2 reso;
    // added 8 bytes for alignment
    glm::vec2 _padding;
};

// used in canvas renderer and canvas class
struct CanvasUtils {
    static inline VkDescriptorSet canvas_texture = VK_NULL_HANDLE;
};

// used in canvas renderer and text editor wrapper class
struct TextEditorUtils {
    static inline bool file_save = false;
};

// used in build panel and canvas renderer class
struct ShadersCompiled {
    static inline bool compiled = false;
};
