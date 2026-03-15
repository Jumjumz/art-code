#include "window.hpp"

Window::Window(const uint32_t width, const uint32_t aspect)
    : width(width), aspect(aspect) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // get height
    this->height = uint32_t(width / aspect);

    // make sure height is not negative
    this->height = (this->height < 1) ? 1 : this->height;

    this->app_window =
        glfwCreateWindow(this->width, this->height, "Art Code", NULL, NULL);
};

void Window::destroy_window() const {
    glfwDestroyWindow(this->app_window);
    glfwTerminate();
};
