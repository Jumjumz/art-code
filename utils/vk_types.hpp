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
    glm::vec2 viewport;
};

// used in canvas renderer and canvas class
struct ArtboardUtils {
    static inline VkDescriptorSet artboard_texture = VK_NULL_HANDLE;
};

// used in canvas renderer and text editor wrapper class
struct TextEditorUtils {
    static inline bool file_save = false;
};

// used in build panel and canvas renderer class
struct ShadersCompiled {
    static inline bool compiled = false;
};

struct CanvasControls {
    static inline float     zoom           = 1.0f;
    static inline glm::vec2 panning        = {0.0f, 0.0f};
    static inline glm::vec2 mouse_last_pos = {0.0f, 0.0f};
};

struct Artboard {
  public:
    static inline void set_artboard_size(const glm::vec2 artboard) {
        Artboard::artboard_size = artboard;
    }

    static inline glm::vec2 get_artboard_size() { return Artboard::artboard_size; }

  private:
    static inline glm::vec2 artboard_size = {};
};
