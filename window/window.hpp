#pragma once

#include <GLFW/glfw3.h>

class Window {
  public:
    Window();

    bool running = true;

    GLFWwindow *app_window;

    void destroy_window() const;

  private:
};
