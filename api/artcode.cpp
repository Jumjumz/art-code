#include "artcode.hpp"
#include "artcode_instance.hpp"
#include <numbers>
#include <stdexcept>

// every resources shared memory needs
struct ShapeAttributes {
    Vec4  color      = Vec4{1.0f};
    Vec2  position   = Vec2{50.0f, 50.0f};
    Vec2  center     = Vec2{50.0f, 50.0f};
    Vec2  shape_data = Vec2{50.0f, 50.0f};
    Vec2  mesh_size  = Vec2{50.0f, 50.0f};
    Vec2  p0         = Vec2{100.0f, 100.0f};
    Vec2  p1         = Vec2{150.0f, 50.0f};
    Vec2  p2         = Vec2{200.0f, 100.0f};
    float stroke     = 10.0f;
    float rotate     = 0.0f;
    int   fill       = 1;
    int   skew       = 0;
    int   shape_type = 0;

    // skew data
    struct ShapeSkew {
        size_t                 size      = 0;
        std::array<SkewPos, 8> skew_data = {{{}}};
    } shape_skew;
    // pen data
    struct PenInstance {
        size_t                   size     = 0;
        std::array<PHandle, 500> pen_data = {{{}}};
    } shape_pen;
};

struct ShapeRegistry {
  public:
    static inline size_t                       size      = 0;
    static inline ArrayT<ShapeAttributes, 500> instances = {};

    static size_t get_size() { return size; }

    static ShapeAttributes get_shape(size_t index) { return instances[index]; }

    static void check_instance_size() {
        if (size > 500)
            throw std::runtime_error(
                "Max number of shapes reach! Max shape intance is 500!");
    }

    static void reset_registry() {
        size = 0;
        std::fill(instances.begin(), instances.end(), ShapeAttributes{});
    }

  private:
};

// init to transparent
Vec4 bg_color = Vec4{0.0f};

enum class ShapeType { Quad, Circle, Triangle, Pen };

// centroid shape_vertices
Vec2 get_center(const ArrayVec2& shape_vertices) {
    Vec2 center = {0.0f, 0.0f};

    for (const auto& v : shape_vertices) {
        center += Vec2{v.x, v.y};
    }
    return center /= static_cast<float>(shape_vertices.size());
};

Vec4 convert_color(const string& color, float opacity) {
    string hex   = color[0] == '#' ? color.substr(1) : color;
    u32    value = std::stoul(hex, nullptr, 16);

    // 0xFF (255) is a bit mask
    // string size of 8 means alpha value is provided
    if (hex.size() == 8) {
        // this means hex is already provided an alpha value which makes the hex size 8 i.e #443199FF
        return Vec4{((value >> 24) & 0xFF) / 255.0f, ((value >> 16) & 0xFF) / 255.0f,
                    ((value >> 8) & 0xFF) / 255.0f, ((value >> 0) & 0xFF) / 255.0f};
    } else {
        // 16 left shift to red pos, 8 left shift to green and blue stay still
        return Vec4{((value >> 16) & 0xFF) / 255.0f, ((value >> 8) & 0xFF) / 255.0f,
                    ((value >> 0) & 0xFF) / 255.0f, opacity};
    }
};

// find length and width of any shapes (forms a quad)
Vec2 mesh_size(const ArrayVec2& shape_vertices) {
    float max_x = -FLT_MAX, min_x = FLT_MAX;
    float max_y = -FLT_MAX, min_y = FLT_MAX;

    for (const auto& vertex : shape_vertices) {
        max_x = glm::max<float>(max_x, vertex.x);
        min_x = glm::min<float>(min_x, vertex.x);
        max_y = glm::max<float>(max_y, vertex.y);
        min_y = glm::min<float>(min_y, vertex.y);
    }

    return Vec2{max_x - min_x, max_y - min_y};
};

// TODO:might need to reduce this to size 4 as 0-4 indices are only needed.. though this is still in consideration
ArrayT<Vec2, 8> get_skew_mesh(const Vec2& mesh_size, const Vec2& shape_pos) {
    // return skew mesh quad
    return {shape_pos,
            shape_pos + Vec2{mesh_size.x * 0.5f, 0.0f},
            shape_pos + Vec2{mesh_size.x, 0.0f},
            shape_pos + Vec2{mesh_size.x, mesh_size.y * 0.5f},
            shape_pos + mesh_size,
            shape_pos + Vec2{mesh_size.x * 0.5f, mesh_size.y},
            shape_pos + Vec2{0.0f, mesh_size.y},
            shape_pos + Vec2{0.0f, mesh_size.y * 0.5f}};
};

ArrayVec2 shape_vertices(ShapeType type, size_t index) {
    ArrayVec2 vertices = {};
    //  quad coordinates and size
    auto& shape_inst = ShapeRegistry::instances[index];
    switch (type) {
    case ShapeType::Quad: {
        return vertices = {
                   shape_inst.position,
                   shape_inst.position + Vec2{shape_inst.shape_data.x, 0.0f},
                   shape_inst.position +
                       Vec2{shape_inst.shape_data.x, shape_inst.shape_data.y},
                   shape_inst.position + Vec2{0.0f, shape_inst.shape_data.y},
        };
        break;
    }
    case ShapeType::Circle: {
        constexpr size_t num_seg = 8;
        for (size_t i = 0; i < num_seg; i++) {
            float angle = i * 2.0f * static_cast<float>(std::numbers::pi) /
                          static_cast<float>(num_seg);

            // radius is both shape_data.x and shape_data.y
            vertices.push_back(
                Vec2{shape_inst.position.x + glm::cos(angle) * shape_inst.shape_data.x,
                     shape_inst.position.y + glm::sin(angle) * shape_inst.shape_data.x});
        }
        return vertices;
        break;
    }
    case ShapeType::Triangle: {
        return vertices = {shape_inst.p0, shape_inst.p1, shape_inst.p2};
        break;
    }
    case ShapeType::Pen: {
        Vec2 position = Vec2(FLT_MAX, FLT_MAX);
        for (size_t i = 0; i < shape_inst.shape_pen.size; i++) {
            Vec2 pos = shape_inst.shape_pen.pen_data[i].position;
            // gets the min of all vertices
            position.x = glm::min<float>(position.x, pos.x);
            position.y = glm::min<float>(position.y, pos.y);
            // push pos
            vertices.push_back(pos);
        }
        // assign position to shape registry
        shape_inst.position = position;
        return vertices;
        break;
    }
    };
};

Vec2 shape_center(int type, size_t index) {
    return get_center(shape_vertices(static_cast<ShapeType>(type), index));
};

Vec2 shape_mesh(int type, size_t index) {
    return mesh_size(shape_vertices(static_cast<ShapeType>(type), index));
};

Skew& Skew::at(int index, Vec2 position) {
    auto& shape = ShapeRegistry::instances[ShapeRegistry::size - 1];
    if (shape.skew == 1) {
        if (this->size < 8) {
            shape.shape_skew.skew_data[this->size] = {position, index};
            // increment sizes
            shape.shape_skew.size++;
            this->size++;
        }
    }
    return *this;
};

using Quad = Art::Quad;

// TODO:implement shapes internals
Quad::Quad()
    : s_index(ShapeRegistry::size),
      _skew(std::make_unique<Skew>()) {
    // increase size
    ShapeRegistry::size++;
    shape_type();
};

void Quad::geometry(float x, float y, float width, float length) {
    ShapeRegistry::instances[this->s_index].position   = Vec2{x, y};
    ShapeRegistry::instances[this->s_index].shape_data = Vec2{width, length};
};

void Quad::geometry(Vec2 position, float width, float length) {
    ShapeRegistry::instances[this->s_index].position   = position;
    ShapeRegistry::instances[this->s_index].shape_data = Vec2{width, length};
};

void Quad::style(bool fill, float stroke, float rotate) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(fill);
    ShapeRegistry::instances[this->s_index].stroke = stroke;
    ShapeRegistry::instances[this->s_index].rotate = rotate;
};

void Quad::style(Style style) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(style.fill);
    ShapeRegistry::instances[this->s_index].stroke = style.stroke;
    ShapeRegistry::instances[this->s_index].rotate = style.rotate;
};

void Quad::color(const char* color, float opacity) {
    ShapeRegistry::instances[this->s_index].color = convert_color(color, opacity);
};

Skew& Quad::skew(bool skew) {
    ShapeRegistry::instances[this->s_index].skew = static_cast<int>(skew);
    return *this->_skew;
};

void Quad::shape_type() {
    ShapeRegistry::instances[this->s_index].shape_type = static_cast<int>(ShapeType::Quad);
};

using Circle = Art::Circle;

Circle::Circle()
    : s_index(ShapeRegistry::size),
      _skew(std::make_unique<Skew>()) {
    ShapeRegistry::size++;
    shape_type();
};

void Circle::geometry(float x, float y, float radius) {
    ShapeRegistry::instances[this->s_index].position   = Vec2{x, y};
    ShapeRegistry::instances[this->s_index].shape_data = Vec2{radius};
};

void Circle::geometry(Vec2 position, float radius) {
    ShapeRegistry::instances[this->s_index].position   = position;
    ShapeRegistry::instances[this->s_index].shape_data = Vec2{radius};
};

void Circle::style(bool fill, float stroke, float rotate) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(fill);
    ShapeRegistry::instances[this->s_index].stroke = stroke;
    ShapeRegistry::instances[this->s_index].rotate = rotate;
};

void Circle::style(Style style) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(style.fill);
    ShapeRegistry::instances[this->s_index].stroke = style.stroke;
    ShapeRegistry::instances[this->s_index].rotate = style.rotate;
};

void Circle::color(const char* color, float opacity) {
    ShapeRegistry::instances[this->s_index].color = convert_color(color, opacity);
};

Skew& Circle::skew(bool skew) {
    ShapeRegistry::instances[this->s_index].skew = static_cast<int>(skew);
    return *this->_skew;
};

void Circle::shape_type() {
    ShapeRegistry::instances[this->s_index].shape_type =
        static_cast<int>(ShapeType::Circle);
};

using Triangle = Art::Triangle;

Triangle::Triangle()
    : s_index(ShapeRegistry::size),
      _skew(std::make_unique<Skew>()) {
    ShapeRegistry::size++;
    shape_type();
};

void Triangle::geometry(Vec2 v0, Vec2 v1, Vec2 v2) {
    // mesh starts at min vector
    const Vec2 position = {glm::min<float>(glm::min<float>(v0.x, v1.x), v2.x),
                           glm::min<float>(glm::min<float>(v0.y, v1.y), v2.y)};

    ShapeRegistry::instances[this->s_index].position = position;
    ShapeRegistry::instances[this->s_index].p0       = v0;
    ShapeRegistry::instances[this->s_index].p1       = v1;
    ShapeRegistry::instances[this->s_index].p2       = v2;
};

void Triangle::style(bool fill, float stroke, float rotate) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(fill);
    ShapeRegistry::instances[this->s_index].stroke = stroke;
    ShapeRegistry::instances[this->s_index].rotate = rotate;
};

void Triangle::style(Style style) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(style.fill);
    ShapeRegistry::instances[this->s_index].stroke = style.stroke;
    ShapeRegistry::instances[this->s_index].rotate = style.rotate;
};

void Triangle::color(const char* color, float opacity) {
    ShapeRegistry::instances[this->s_index].color = convert_color(color, opacity);
};

Skew& Triangle::skew(bool skew) {
    ShapeRegistry::instances[this->s_index].skew = static_cast<int>(skew);
    return *this->_skew;
};

void Triangle::shape_type() {
    ShapeRegistry::instances[this->s_index].shape_type =
        static_cast<int>(ShapeType::Triangle);
};

using Pen = Art::Pen;

Pen::Pen()
    : s_index(ShapeRegistry::size),
      _skew(std::make_unique<Skew>()) {
    ShapeRegistry::size++;
    shape_type();
};

// TODO:implement pen implementations
void Pen::point(float x, float y) {
    auto& pen = ShapeRegistry::instances[this->s_index].shape_pen;
    if (pen.size < 500) {
        // id = 1 indicates begin/end points is a segment
        pen.pen_data[pen.size].position = Vec2{x, y};
        pen.pen_data[pen.size].id       = 1;
        pen.size++;
    }
};

void Pen::curve(float h_x, float h_y) {
    constexpr int ID     = 2;
    const Vec2    HANDLE = {h_x, h_y};

    auto& pen = ShapeRegistry::instances[this->s_index].shape_pen;
    if (pen.size < 500) {
        // NOTE:this is added so that we dont add array in index -1
        // this is for situations where curve is called first
        // check if we are at the beginning of the array
        if (pen.size == 0) {
            pen.pen_data[pen.size].position = HANDLE;
            // id = 2 is the curve indicator
            pen.pen_data[pen.size].id = ID;
        } else {
            // updated the prev index .id to 0
            // 0 is the indicator that the next point is the HANDLE
            pen.pen_data[pen.size - 1].id = 0;
            // add handle to current index
            pen.pen_data[pen.size].position = HANDLE;
            // id = 2 is the curve indicator
            pen.pen_data[pen.size].id = ID;
        }
        pen.size++;
    }
};

void Pen::style(bool fill, float stroke, float rotate) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(fill);
    ShapeRegistry::instances[this->s_index].stroke = stroke;
    ShapeRegistry::instances[this->s_index].rotate = rotate;
};

void Pen::style(Style style) {
    ShapeRegistry::instances[this->s_index].fill   = static_cast<int>(style.fill);
    ShapeRegistry::instances[this->s_index].stroke = style.stroke;
    ShapeRegistry::instances[this->s_index].rotate = style.rotate;
};

void Pen::color(const char* color, float opacity) {
    ShapeRegistry::instances[this->s_index].color = convert_color(color, opacity);
};

Skew& Pen::skew(bool skew) {
    ShapeRegistry::instances[this->s_index].skew = static_cast<int>(skew);
    return *this->_skew;
};

void Pen::shape_type() {
    ShapeRegistry::instances[this->s_index].shape_type = static_cast<int>(ShapeType::Pen);
};

//  background color
void Art::BackgroundColor(const char* color, float opacity) {
    bg_color = convert_color(color, opacity);
};

void Art::Draw() {
    // load shared memory
    Shared::Memory::load_shared_memory();
    {
        // check size first
        ShapeRegistry::check_instance_size();

        const auto shape_size = ShapeRegistry::get_size();

        for (size_t i = 0; i < shape_size; i++) {
            const auto& shape = ShapeRegistry::instances[i];

            const auto shape_type = shape.shape_type;
            const auto mesh_size  = shape_mesh(shape_type, i);

            PushConstants constants{};
            constants.bg_color   = bg_color;
            constants.color      = shape.color;
            constants.pos        = shape.position;
            constants.center     = shape.center; // not used
            constants.shape_data = shape.shape_data;
            constants.mesh_size  = mesh_size;
            // triangle uses these in shaders instead of shape data
            constants.p0 = shape.p0;
            constants.p1 = shape.p1;
            constants.p2 = shape.p2;

            constants.stroke     = shape.stroke;
            constants.rotate     = shape.rotate;
            constants.fill       = shape.fill;
            constants.skew       = shape.skew;
            constants.shape_type = shape_type;

            Shared::Memory::register_constants(constants);

            Shared::Memory::register_pen_data(shape.shape_pen.pen_data,
                                              shape.shape_pen.size);

            const auto& skew_mesh = get_skew_mesh(constants.mesh_size, constants.pos);

            SkewData skew_data{};
            skew_data.skew_mesh = skew_mesh;
            skew_data.skew_pos  = shape.shape_skew.skew_data;

            Shared::Memory::register_skew_data(skew_data, shape.shape_skew.size);

            // increment instance size, mutates instance size per loop execution
            Shared::Memory::increment_instance_size();
        }
        // reset registry after transfering data to shared memory
        ShapeRegistry::reset_registry();
    }
};
