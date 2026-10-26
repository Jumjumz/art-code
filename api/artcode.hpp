#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#ifdef API_EXPORTS
#define ARTCODE_API __declspec(dllexport)
#else
#define ARTCODE_API __declspec(dllimport)
#endif
#else
#define ARTCODE_API
#endif

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

template <typename T, size_t size> using ArrayT = std::array<T, size>;
template <typename T> using VectorT             = std::vector<T>;
// TODO:currently this can receive chars or strings, update to only receive NUMBERS
//  squared and cubic
template <typename T> T squared(T sq) { return sq * sq; };
template <typename T> T cubic(T cb) { return cb * cb * cb; };

struct ARTCODE_API Skew {
  public:
    Skew& at(int index, Vec2 position);

  private:
    size_t size;
};

struct Style {
    bool  fill   = false;
    float stroke = 10.0f;
    float rotate = 0.0f;
};

namespace Art {
    class ARTCODE_API Quad {
      private:
        size_t                s_index;
        std::unique_ptr<Skew> _skew;

        void shape_type();

      public:
        Quad();

        void  geometry(float x, float y, float width, float length);
        void  geometry(Vec2 position, float width, float length);
        void  style(bool fill = true, float stroke = 10.0f, float rotate = 0.0f);
        void  style(Style style);
        void  color(const char* color = "#000000", float opacity = 1.0f);
        Skew& skew(bool skew = false);
    };

    class ARTCODE_API Circle {
      private:
        size_t                s_index;
        std::unique_ptr<Skew> _skew;

        void shape_type();

      public:
        Circle();

        void  geometry(float x, float y, float radius);
        void  geometry(Vec2 position, float radius);
        void  style(bool fill = true, float stroke = 10.0f, float rotate = 0.0f);
        void  style(Style style);
        void  color(const char* color = "#000000", float opacity = 1.0f);
        Skew& skew(bool skew = false);
    };

    class ARTCODE_API Triangle {
      private:
        size_t                s_index;
        std::unique_ptr<Skew> _skew;

        void shape_type();

      public:
        Triangle();

        void  geometry(Vec2 v0, Vec2 v1, Vec2 v2);
        void  style(bool fill = true, float stroke = 10.0f, float rotate = 0.0f);
        void  style(Style style);
        void  color(const char* color = "#000000", float opacity = 1.0f);
        Skew& skew(bool skew = false);
    };

    class ARTCODE_API Pen {
      private:
        size_t                s_index;
        std::unique_ptr<Skew> _skew;

        void shape_type();

      public:
        Pen();

        // position and handle
        void point(float x, float y);
        void curve(float h_x, float h_y);

        void  style(bool fill = false, float stroke = 10.0f, float rotate = 0.0f);
        void  style(Style style);
        void  color(const char* color, float opacity = 1.0f);
        Skew& skew(bool skew = false);
    };

    ARTCODE_API void Draw();

    ARTCODE_API void BackgroundColor(const char* color, float opacity = 1.0f);
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
