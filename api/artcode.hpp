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

// TODO:currently this can receive chars or strings, update to only receive NUMBERS
//  squared and cubic
template <typename T> T squared(T sq) { return sq * sq; };
template <typename T> T cubic(T cb) { return cb * cb * cb; };

// TODO:add predefined positions such as center, corner etc.

enum class TriangleTypes { Equilateral, Right, FreeForm };

// hides padding as this is for alingment in the gpu
struct SkewPos {
    Vec2 pos;
    int  index;

    SkewPos()
        : pos{},
          index(0),
          _padding(0) {}
    SkewPos(Vec2 p, int i)
        : pos{p},
          index{i},
          _padding{0} {}

  private:
    int _padding;
};

struct Handles {
    bool handle;
    Vec2 handlePosition;

    Handles()
        : handle(false),
          handlePosition(0, 0) {}
    Handles(bool h, Vec2 hp)
        : handle(h),
          handlePosition(hp) {}
};

// TODO:replace struct name, can be more general name as other shapes will use it
struct PenHandles {
    Vec2    position;
    Handles handles;
};

template <typename T, size_t size> using ArrayT = std::array<T, size>;
template <typename T> using VectorT             = std::vector<T>;

// TODO:remove vertices and indices generation funcions
namespace detail {
    class IPen {
      public:
        virtual ~IPen() = default;

        IPen()
            : position(200, 200),
              color("#000000"),
              stroke(1.0f),
              rotate(0.0f),
              opacity(1.0f),
              fill(true),
              skew(false),
              skewPos({}) {}

        // TODO:make skewPos a function.. or change the way skew pos syntax wise
        // uses camel case for users, snake case for api implementation
        // TODO: replace position with PenHeandles struct, as in the future a warp feature will be added
        Vec2  position;
        Color color;
        float stroke;
        float rotate;
        float opacity;
        bool  fill;
        bool  skew;
        // TODO:this is atrocius to write in user land, make this a struct that is
        // comprehendable to write and read
        ArrayT<SkewPos, 8> skewPos;

        // must implement
        virtual ArrayVec4 generate_vertices() const = 0;
        virtual ArrayU32  generate_indices() const  = 0;
        // NOTE:new functions for sdf to work
        virtual Vec2 shape_data() const = 0;
        virtual int  shape_type() const = 0;

        // centroid vertices
        Vec2 get_center() const {
            Vec2 center = {0.0f, 0.0f};

            const auto& verts = this->generate_vertices();
            for (const auto& v : verts) {
                center += Vec2{v.x, v.y};
            }
            return center /= static_cast<float>(verts.size());
        };
    };
} // namespace detail

namespace Art {
    class Quad : public detail::IPen {
      public:
        Quad();

        float l, w;

      private:
        ArrayVec4 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
        Vec2      shape_data() const override;
        int       shape_type() const override;
    };

    class Circle : public detail::IPen {
      public:
        Circle();

        float radius;

      private:
        ArrayVec4 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
        Vec2      shape_data() const override;
        int       shape_type() const override;
        // num of triangles to make a circle, also defines the smoothness
        size_t get_num_vert() const;
    };

    class Triangle : public detail::IPen {
      public:
        Triangle();

        // TODO:equilateral only needs base, find a way for the compiler to provide a
        // warning if hegiht is being used by the user
        float base, height;
        // for free form
        Vec2 p0, p1, p2;

        TriangleTypes type;

      private:
        ArrayVec4 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
        Vec2      shape_data() const override;
        int       shape_type() const override;
    };

    class Pen : public detail::IPen {
      public:
        Pen();

        VectorT<PenHandles> positions;

      private:
        ArrayVec4 generate_vertices() const override;
        ArrayU32  generate_indices() const override;
        Vec2      shape_data() const override;
        int       shape_type() const override;
    };

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
