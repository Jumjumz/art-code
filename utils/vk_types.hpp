#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// for viewing/displaying
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

// artboard
struct ArtboardBuffer {
    glm::mat4 proj;
    glm::mat4 view;
    glm::vec4 color;
    glm::vec2 reso;
    glm::vec2 mouse;
};

struct CanvasUtils {
    // use in art code and canvas class
    inline static VkDescriptorSet canvas_texture = VK_NULL_HANDLE;
    inline static glm::vec3 camera = {0, 0, 0};
};
