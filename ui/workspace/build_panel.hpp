#pragma once

#include <string>

class BuildPanel {
  public:
    BuildPanel();

    void render();

  private:
    /* TODO: implement progress bar
      std::atomic<bool> show_progress = false;
      std::atomic<float> progress = 0.0f;
    */
    bool project_compiled = false;

    std::string executable_files() const;

    void add_includes() const;

    // C for compile, R for Run
    enum class Flags { C, R };

    std::string create_cmd(const Flags &flag);

    std::string execute(const std::string &cmd) const;
};
