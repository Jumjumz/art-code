#pragma once

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

typedef Vec4 Color;
// typedef char* Colour;
// Arrays
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

namespace detail {
    struct IPen {
        virtual ~IPen() = default;

        // must implement
        Vec2 position;
        // TODO:make this a hex color, where it converts to vec4 to the GPU
        Color color;
        // Colour colour[5];
        float stroke;
        float rotate;
        bool  fill;

        virtual ArrayVec2 generate_vertices() const = 0;
        virtual ArrayU32  generate_indices() const  = 0;

        // centroid vertices
        Vec2 get_center() const {
            Vec2 center = {0.0f, 0.0f};
            for (const auto& v : this->generate_vertices()) {
                center += v;
            }
            return center /= static_cast<float>(this->generate_vertices().size());
        };
        // TODO:add a hex string to vec4 converter function
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

        float size;

        enum class TriangleTypes { RIGHT, ACUTE, OBTUSE, EQUILATERAL };

      private:
        ArrayVec2 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
    };
    // TODO:add pen tool

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
