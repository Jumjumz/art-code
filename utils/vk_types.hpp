#pragma once

#include <glm/glm.hpp>

// for viewing/displaying
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
