#pragma once

class WorkSpace {
  public:
    virtual ~WorkSpace() = default;

    virtual void render() = 0;

  protected:
  private:
};
