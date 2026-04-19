#pragma once

#include <glm/fwd.hpp>

#include <memory>

// Forward declarations
// build system
struct Sources;

// artcode
struct IPen;
struct PenTool;
struct Triangle;
struct Square;
struct Circle;

// Vectors data types
typedef glm::vec1 Vec1;
typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;

// sources
struct Sources {
  public:
    Sources();

    ~Sources();

    void add();

    void build();

  private:
    struct Source;
    std::unique_ptr<Source> _sources;
};
