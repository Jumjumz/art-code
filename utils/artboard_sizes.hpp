#pragma once

#include <glm/glm.hpp>
#include <string>
#include <tuple>
#include <vector>

enum class TemplateSizes { WEB_COMMON, WEB_LARGE, WEB_MEDIUM, WEB_MINIMUM };

using Templates = std::tuple<const std::string, const TemplateSizes>;

struct ArtboardTemplateSize {
  private:
    static inline glm::vec3 artboard_size;
    static inline bool show = false;

  public:
    static inline bool artboard_show() { return show; };

    static inline glm::vec3 artboard_get_size() {
        return ArtboardTemplateSize::artboard_size;
    };

    static inline void artboard_set_size(const TemplateSizes &temp) {
        switch (temp) {
        case TemplateSizes::WEB_COMMON: {
            artboard_size = {1366.0f, 768.0f, 72.0f};
            show = true;
            break;
        }
        case TemplateSizes::WEB_LARGE: {
            artboard_size = {1920.0f, 1080.0f, 72.0f};
            show = true;
            break;
        }
        case TemplateSizes::WEB_MEDIUM: {
            artboard_size = {1440.0f, 900.0f, 72.0f};
            show = true;
            break;
        }
        case TemplateSizes::WEB_MINIMUM: {
            artboard_size = {1024.0f, 768.0f, 72.0f};
            show = true;
            break;
        }
        default:
            artboard_size = {1367.0f, 768.0f, 72.0f};
            show = true;
            break;
        }
    };

    static inline void artboard_custom_size(const float &width,
                                            const float &height,
                                            const float &ppi) {
        artboard_size = {width, height, ppi};
        show = true;
    }
};

struct ArtboardSize {
    static inline float width;
    static inline float height;
    static inline float ppi;
};

struct ArtboardTemplates {
    static inline const std::vector<Templates> TEMPLATES = {
        {"Web Most Common", TemplateSizes::WEB_COMMON},
        {"Web Large", TemplateSizes::WEB_LARGE},
        {"Web Medium", TemplateSizes::WEB_MEDIUM},
        {"Web Minimum", TemplateSizes::WEB_MINIMUM}};
};
