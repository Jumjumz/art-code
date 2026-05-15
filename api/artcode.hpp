#pragma once

#include <filesystem>
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

typedef glm::vec3 Color;

template <typename to_string> string ToString(const to_string& to_str) {
    return std::to_string(to_str);
};
// filesystem
namespace fs = std::filesystem;

inline const fs::path PROJECT_DIR =
    fs::canonical("/proc/self/exe").parent_path().parent_path();

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

          protected:
            // shader lines of init, idx that contains version and layout keywords 0 -> 3
            static constexpr int SHADER_DECLARATIONS_IDX = 3;
            // num of times derived class is initialize
            static inline int init_count = 0;

            fs::path shader_file() const {
                return PROJECT_DIR / "shaders" / "artcode.frag";
            };
            virtual string to_glsl() const                       = 0;
            virtual void   write_shader(const string& glsl_code) = 0;
            virtual void   draw()                                = 0;
        };
    } // namespace detail

    struct Circle : detail::IPen {
        Circle();

        float radius;

        void draw() override;

      private:
        string to_glsl() const override;

        void write_shader(const string& glsl_code) override;
    };
    // TODO:add others

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
