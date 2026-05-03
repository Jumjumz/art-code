#pragma once

#include <glm/fwd.hpp>

#include <string>
#include <vector>

// Forward declarations
// artcode
class IPen;
class PenTool;
class Triangle;
class Square;
class Circle;
// structs
struct Impl;

// Arrays
typedef std::vector<int> ArrayInt;
typedef std::vector<float> ArrayFloat;
typedef std::vector<double> ArrayDouble;
typedef std::vector<std::string> ArrayString;

// Vectors data types
typedef glm::vec1 Vec1;
typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;

class IPen {
  public:
    virtual ~IPen() = default;

  protected:
    // must implement
    virtual IPen &position(const Vec2 &position) const = 0;
    virtual IPen &scale(const Vec2 &scale) const = 0;
    virtual IPen &handle(const Vec2 &handle) const = 0;
    virtual void draw() const = 0;
    // optional
    // only for shapes
};

// Draw::PenTool("upper-left")->position()->draw();
