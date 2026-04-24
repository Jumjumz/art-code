#pragma once

#include <atomic>
#include <string>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    std::atomic<bool> show_progress = false;
    std::atomic<float> progress = 0.0f;

    std::string executable_files() const;

    void add_includes() const;

    // C for compile, R for Run
    enum class Flags { C, R };

    std::string execute(const Flags &flag);
};
