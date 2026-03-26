#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// for viewing/displaying
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct CanvasUtils {
    // use in art code and canvas class
    inline static VkDescriptorSet canvas_texture = VK_NULL_HANDLE;
};
