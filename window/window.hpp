#ifndef WINDOW_HPP
#define WINDOW_HPP

#pragma once

#include <GLFW/glfw3.h>

class Window {
  public:
    Window(const uint32_t width, const uint32_t aspect);

    uint32_t width;
    uint32_t aspect;
    bool running = true;

    GLFWwindow *app_window;

    void destroy_window() const;

  private:
    uint32_t height;
};

#endif // !WINDOW_HPP
