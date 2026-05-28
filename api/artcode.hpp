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

// Arrays
typedef std::vector<int>       ArrayInt;
typedef std::vector<float>     ArrayFloat;
typedef std::vector<double>    ArrayDouble;
typedef std::vector<string>    ArrayString;
typedef std::vector<glm::vec1> ArrayVec1;
typedef std::vector<glm::vec2> ArrayVec2;
typedef std::vector<glm::vec3> ArrayVec3;
typedef std::vector<glm::vec4> ArrayVec4;

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

        virtual void generate_vertices() = 0;
    };
} // namespace detail

namespace Art {
    struct Quad : detail::IPen {
      public:
        Quad();

        ~Quad();

        float l, w;

      private:
        static constexpr u32 num_vert = 4;
        void                 generate_vertices() override;
    };

    struct Circle : detail::IPen {
      public:
        Circle();

        ~Circle();

        float radius;

      private:
        static constexpr u32 num_vert = 8;
        void                 generate_vertices() override;
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
