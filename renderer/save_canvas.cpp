#include "save_canvas.hpp"

SaveCanvas::SaveCanvas() {};

void SaveCanvas::save_canvas(const vk::raii::DeviceMemory& memory) {
    // * 4 for RGBA
    vk::DeviceSize image_size = this->artboard.x * this->artboard.y * 4;
};
