#include "art_code.hpp"

ArtCode::ArtCode() {};

void ArtCode::run() {
    loop();
    cleanup();
};

void ArtCode::loop() {
    while (!glfwWindowShouldClose(this->window.app_window)) {
        glfwPollEvents();
    }
};

void ArtCode::cleanup() const { this->window.destroy_window(); }
