#pragma once

#include <array>
#include <glm/glm.hpp>

#include <string>
#include <vector>

// strings
typedef std::string string;

// unsinged int
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// glm data types
typedef glm::vec1 Vec1;
typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;

typedef string Color;
// Arrays/Vector
typedef std::vector<int>    ArrayInt;
typedef std::vector<float>  ArrayFloat;
typedef std::vector<double> ArrayDouble;
typedef std::vector<string> ArrayString;
typedef std::vector<Vec1>   ArrayVec1;
typedef std::vector<Vec2>   ArrayVec2;
typedef std::vector<Vec3>   ArrayVec3;
typedef std::vector<Vec4>   ArrayVec4;
typedef std::vector<u8>     ArrayU8;
typedef std::vector<u16>    ArrayU16;
typedef std::vector<u32>    ArrayU32;
typedef std::vector<u64>    ArrayU64;

// TODO:add predefined positions such as center, corner etc.

enum class TriangleTypes { Equilateral, Right };

struct SkewPos {
    int   index;
    float angle;
};

template <typename T, uint32_t size> using ArrayT = std::array<T, size>;

// TODO:add skew for shapes
namespace detail {
    struct IPen {
        virtual ~IPen() = default;

        // TODO:make multiple skew points for diff shapes
        //  must implement
        //  uses camel case for users, snake case for api implementation
        Vec2               position;
        Color              color;
        float              stroke;
        float              rotate;
        float              opacity;
        bool               fill;
        bool               skew;
        ArrayT<SkewPos, 8> skewPos = {};

        virtual ArrayVec2 generate_vertices() const = 0;
        virtual ArrayU32  generate_indices() const  = 0;

        // centroid vertices
        Vec2 get_center() const {
            Vec2 center = {0.0f, 0.0f};

            const auto& verts = this->generate_vertices();
            for (const auto& v : verts) {
                center += v;
            }
            return center /= static_cast<float>(verts.size());
        };
    };
} // namespace detail

namespace Art {
    struct Quad : detail::IPen {
      public:
        Quad();

        ~Quad();

        float l, w;

      private:
        ArrayVec2 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
    };

    struct Circle : detail::IPen {
      public:
        Circle();

        ~Circle();

        float radius;

      private:
        // num of triangles to make a circle, also defines the smoothness
        static constexpr int SEGMENTS = 32;

        ArrayVec2 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
    };

    struct Triangle : detail::IPen {
      public:
        Triangle();

        ~Triangle();

        float         size;
        TriangleTypes type;

      private:
        ArrayVec2 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
    };
    // TODO:add pen tool

    void Draw();
}; // namespace Art

// TODO:add expresssion templates
/* STILL LEARNING THIS SHIT
template <typename Drawable> struct Draw {
  private:
    Drawable drawable;
    std::string glsl_code;

  public:
    void draw() { this->drawable.glsl(); };
};
*/
