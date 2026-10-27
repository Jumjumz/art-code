#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// artboard
struct ArtboardBuffer {
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
    glm::vec2 reso;
    float     ppi;
    float     zoom;
};

// used in canvas renderer and canvas class
struct ArtboardUtils {
    static inline VkDescriptorSet artboard_texture = VK_NULL_HANDLE;
};

// used in canvas renderer and text editor wrapper class
struct TextEditorUtils {
    static inline bool is_file_save = false;
};

// used in build panel and canvas renderer class
struct ShadersCompiled {
    static inline bool is_compiled = false;
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

    static inline void set_artboard_zoom(const glm::vec2 artboard_zoom) {
        Artboard::artboard_zoom = artboard_zoom;
    }

    static inline glm::vec2 get_artboard_size() { return Artboard::artboard_size; }

    static inline glm::vec2 get_artboard_zoom() { return Artboard::artboard_zoom; }

  private:
    static inline glm::vec2 artboard_size = {0.0f, 0.0f};
    static inline glm::vec2 artboard_zoom = {0.0f, 0.0f};
};
