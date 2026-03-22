#pragma once

class StartScreen {
  public:
    virtual ~StartScreen() = default;

    virtual void render() = 0;

  protected:
  private:
};

class WorkSpace {
  public:
    virtual ~WorkSpace() = default;

    virtual void render() = 0;

  protected:
  private:
};
