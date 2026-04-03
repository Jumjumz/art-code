#pragma once

#include <GLFW/glfw3.h>

class Window {
  public:
    Window();

    GLFWwindow *app_window;

    void destroy_window() const;

  private:
};
