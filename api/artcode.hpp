#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

// Forward declarations
// artcode
struct IPen;
struct PenTool;
struct Triangle;
struct Square;
struct Circle;

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

// Colors
typedef glm::vec3 Color;

namespace Art {
namespace detail {
struct IPen {
    virtual ~IPen() = default;

    // must implement
    Vec2 position;

    Color color;

    float stroke;

    float scale;

    virtual void draw() = 0;
    // optional
    // only for shapes
};
} // namespace detail

struct Circle : detail::IPen {
    Circle();

    float radius;

    void draw() override;
};
// TODO:add others

}; // namespace Art

// Draw::Pen("upper-left")->position()->render();
// Draw::Pen pen_tool;
// pen_tool.position = {100, 100};
// pen_tool.render();
