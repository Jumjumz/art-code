#include "art_code.hpp"

ArtCode::ArtCode() {};

void ArtCode::run() {
    loop();
    cleanup();
};

void ArtCode::loop() {
    while (!glfwWindowShouldClose(this->window.app_window)) {
        this->window.running = true;
    }
};

void ArtCode::cleanup() const { this->window.destroy_window(); }
