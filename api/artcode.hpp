#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

// Forward declarations
// artcode
struct IPen;
struct PenTool;
struct Triangle;
struct Quad;
struct Circle;

// shape/class register
struct ShapeRegistry;
struct InstanceTracking;

// strings
typedef std::string string;

// Arrays
typedef std::vector<int>    ArrayInt;
typedef std::vector<float>  ArrayFloat;
typedef std::vector<double> ArrayDouble;
typedef std::vector<string> ArrayString;

// glm data types
typedef glm::vec1 Vec1;
typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;

typedef Vec4 Color;

namespace detail {
    struct IPen {
        virtual ~IPen() = default;

        // must implement
        Vec2  position;
        Color color;
        float stroke;
        float scale;
    };
} // namespace detail

namespace Art {
    struct Circle : detail::IPen {
      public:
        Circle();

        ~Circle();

        float radius;

      private:
    };

    struct Quad : detail::IPen {
      public:
        Quad();

        ~Quad();

        float l, w;

      private:
    };
    // TODO:add others

    void Draw();
}; // namespace Art

/* STILL LEARNING THIS SHIT
template <typename Drawable> struct Draw {
  private:
    Drawable drawable;
    std::string glsl_code;

  public:
    void draw() { this->drawable.glsl(); };
};
*/
