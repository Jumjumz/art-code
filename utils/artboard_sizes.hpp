#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

enum class TemplateSizes : std::uint32_t {
    WEB_COMMON,
    WEB_LARGE,
    WEB_MEDIUM,
    WEB_MINIMUM
};

using Templates = std::tuple<const std::string, const TemplateSizes>;

struct ArtboardTemplates {
    static inline const std::vector<Templates> TEMPLATES = {
        {"Web Most Common", TemplateSizes::WEB_COMMON},
        {"Web Large", TemplateSizes::WEB_LARGE},
        {"Web Medium", TemplateSizes::WEB_MEDIUM},
        {"Web Minimum", TemplateSizes::WEB_MINIMUM}};
};
