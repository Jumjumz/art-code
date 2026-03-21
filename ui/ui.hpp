#pragma once

class UI {
  public:
    virtual ~UI() = default;

    virtual void render() = 0;

  protected:
  private:
};
