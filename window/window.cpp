#include "window.hpp"

Window::Window() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    this->aspect_ratio = float(mode->width) / mode->height;

    this->app_window =
        glfwCreateWindow(mode->width, mode->height, "Art Code", NULL, NULL);
};

void Window::destroy_window() const {
    glfwDestroyWindow(this->app_window);
    glfwTerminate();
};
