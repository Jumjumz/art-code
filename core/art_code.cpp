#include "art_code.hpp"

ArtCode::ArtCode() {};

void ArtCode::run() {
    while (!glfwWindowShouldClose(this->window.app_window)) {
        this->window.running = true;
    }

    cleanup();
};

void ArtCode::cleanup() const { this->window.destroy_window(); }
