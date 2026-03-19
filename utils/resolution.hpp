#ifndef RESOLUTION_HPP
#define RESOLUTION_HPP

#pragma once

#include <cstdint>

struct WindowSize {
    enum class Resolution { HD, FHD, QHD };

    enum class Aspect { R4_3, R16_9, R16_10 };

    static uint32_t get_resolution(const Resolution &res) {
        switch (res) {
        case Resolution::HD: {
            return 1280;
        }
        case Resolution::FHD: {
            return 1920;
        }
        case Resolution::QHD: {
            return 2560;
        }
        default: {
            return 1920;
        }
        }
    };

    static float get_aspect(const Aspect &aspect) {
        switch (aspect) {
        case Aspect::R4_3: {
            return 4.0f / 3.0f;
        }
        case Aspect::R16_9: {
            return 16.0f / 9.0f;
        }
        case Aspect::R16_10: {
            return 16.0f / 10.0f;
        }
        default: {
            return 16.0f / 9.0f;
        }
        }
    };
};

#endif // !RESOLUTION_HPP
