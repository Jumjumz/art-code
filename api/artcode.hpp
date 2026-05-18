#pragma once

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations
// artcode
struct IPen;
struct PenTool;
struct Triangle;
struct Square;
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

// others
typedef std::unordered_map<int, string> UMap;

template <typename to_string> string ToString(const to_string& to_str) {
    return std::to_string(to_str);
};

// TODO:redo api design
namespace Art {
    namespace detail {
        struct IPen {
            virtual ~IPen() = default;

            // must implement
            string name;
            Vec2   position;
            Color  color;
            float  stroke;
            float  scale;

            virtual string to_glsl_func() const = 0;
            virtual string to_glsl_var() const  = 0;
        };
    } // namespace detail

    struct Circle : detail::IPen {
        Circle();

        ~Circle();

        float radius;

      private:
        string to_glsl_func() const override;
        string to_glsl_var() const override;
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
