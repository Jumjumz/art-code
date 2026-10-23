#include "artcode.hpp"
#include "artcode_instance.hpp"
#include <cmath>
#include <numbers>
#include <stdexcept>

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
        max_x = std::max<float>(max_x, vertex.x);
        min_x = std::min<float>(min_x, vertex.x);
        max_y = std::max<float>(max_y, vertex.y);
        min_y = std::min<float>(min_y, vertex.y);
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
                Vec2{shape_inst.position.x + std::cos(angle) * shape_inst.shape_data.x,
                     shape_inst.position.y + std::sin(angle) * shape_inst.shape_data.x});
        }
        return vertices;
        break;
    }
    case ShapeType::Triangle: {
        return vertices = {shape_inst.p0, shape_inst.p1, shape_inst.p2};
        break;
    }
    case ShapeType::Pen: {
        return vertices = {};
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
            shape.shape_skew.skew_data[this->size] = {index, position};
            // increment sizes
            shape.shape_skew.size++;
            this->size++;
        }
    }
    return *this;
};

using Quad     = Art::Quad;
using Circle   = Art::Circle;
using Triangle = Art::Triangle;
using Pen      = Art::Pen;

// TODO:implement shapes internals
Quad::Quad()
    : s_index(ShapeRegistry::size),
      _skew(std::make_unique<Skew>()) {
    // increase size
    ShapeRegistry::size++;
    shape_type();
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

Circle::Circle()
    : s_index(ShapeRegistry::size),
      _skew(std::make_unique<Skew>()) {
    ShapeRegistry::size++;
    shape_type();
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

void Circle::color(const char* color, float opacity) {
    ShapeRegistry::instances[this->s_index].color = convert_color(color, opacity);
};

Skew& Circle::skew(bool skew) {
    ShapeRegistry::instances[this->s_index].skew = skew;
    return *this->_skew;
};

void Circle::shape_type() {
    ShapeRegistry::instances[this->s_index].shape_type =
        static_cast<int>(ShapeType::Circle);
};

Triangle::Triangle() {
    ShapeRegistry::size++;
    shape_type();
};

struct Triangle::Intrls {
    int  skew = 0;
    int  index;
    Vec2 position;
};

void Triangle::geometry(Vec2 v0, Vec2 v1, Vec2 v2) {
    const auto index = ShapeRegistry::size - 1;
    // mesh starts at min vector
    const Vec2 position = {std::min<float>({v0.x, v1.x, v2.x}),
                           std::min<float>({v0.y, v1.y, v2.y})};

    ShapeRegistry::instances[index].position = position;
    ShapeRegistry::instances[index].p0       = v0;
    ShapeRegistry::instances[index].p1       = v1;
    ShapeRegistry::instances[index].p2       = v2;
};

void Triangle::style(bool fill, float stroke, float rotate) {
    const auto index = ShapeRegistry::size - 1;

    ShapeRegistry::instances[index].fill   = static_cast<int>(fill);
    ShapeRegistry::instances[index].stroke = stroke;
    ShapeRegistry::instances[index].rotate = rotate;
};

void Triangle::color(const char* color, float opacity) {
    ShapeRegistry::instances[ShapeRegistry::size - 1].color = convert_color(color, opacity);
};

Triangle::Intrls& Triangle::skew(bool skew) {
    this->intrls->skew = skew;

    ShapeRegistry::instances[ShapeRegistry::size - 1].skew = this->intrls->skew;
    return *this->intrls;
};

Triangle::Intrls& Triangle::at(int index, Vec2 position) {
    // TODO:apply skew position
    return *this->intrls;
};

void Triangle::shape_type() {
    ShapeRegistry::instances[ShapeRegistry::size - 1].shape_type =
        static_cast<int>(ShapeType::Triangle);
};

Pen::Pen() {
    ShapeRegistry::size++;
    shape_type();
};

struct Pen::Intrls {
    int  skew;
    int  index;
    Vec2 position;
};

// TODO:implement pen implementations
void Pen::point(Vec2 position) {

};

void Pen::curve(Vec2 handle) {

};

void Pen::style(bool fill, float stroke, float rotate) {

};

Pen::Intrls& Pen::skew(bool skew) { return *this->intrls; };

Pen::Intrls& Pen::at(int index, Vec2 position) { return *this->intrls; };

void Pen::shape_type() {
    ShapeRegistry::instances[ShapeRegistry::size - 1].shape_type =
        static_cast<int>(ShapeType::Pen);
};

// TODO:remove shape_vertices and indices functions
// API implementations
/*using DrawQuad     = Artt::Quad;
using DrawCircle   = Artt::Circle;
using DrawTriangle = Artt::Triangle;
using DrawPen      = Artt::Pen;

// Quad
DrawQuad::Quad()
    : l(100),
      w(100) {
    InstanceRegistry::register_instance(this);
};

ArrayVec2 DrawQuad::generate_vertices() const {
    //  quad coordinates and size
    return ArrayVec2{
        this->position,
        this->position + Vec2{this->w, 0.0f},
        this->position + Vec2{this->w, this->l},
        this->position + Vec2{0.0f, this->l},
    };
};

Vec2 DrawQuad::shape_data() { return Vec2{this->w, this->l}; };

int DrawQuad::shape_type() const { return static_cast<int>(ShapeType::Quad); };

// Circle
DrawCircle::Circle()
    : radius(100.0f) {
    InstanceRegistry::register_instance(this);
};

ArrayVec2 DrawCircle::generate_vertices() const {
    ArrayVec2 vertex;
    // center of the circle
    vertex.push_back(this->position);

    const size_t num_seg = 8;
    for (size_t i = 0; i < num_seg; i++) {
        float angle = i * 2.0f * std::numbers::pi / num_seg;

        vertex.push_back(Vec4{this->position.x + std::cos(angle) * this->radius,
                              this->position.y + std::sin(angle) * this->radius, 2.0f,
                              2.0f});
    }
    return vertex;
};

Vec2 DrawCircle::shape_data() { return Vec2{this->radius, this->radius}; };

int DrawCircle::shape_type() const { return static_cast<int>(ShapeType::Circle); };

// Triangle
DrawTriangle::Triangle() { InstanceRegistry::register_instance(this); };

ArrayVec2 DrawTriangle::generate_vertices() const {
    return ArrayVec2{this->v0, this->v1, this->v2};
};

// NOTE:triangle doesnt need to return shape data
// just needs to assign p0, p1 and p2
Vec2 DrawTriangle::shape_data() {
    this->v0 += this->position;
    this->v1 += this->position;
    this->v2 += this->position;

    return this->v0;
};

int DrawTriangle::shape_type() const { return static_cast<int>(ShapeType::Triangle); };

DrawPen::Pen() {
    // pen is false by default
    this->fill = false;

    InstanceRegistry::register_instance(this);
};

ArrayVec2 DrawPen::generate_vertices() const {
    ArrayVec2 vertex = {};
    vertex.reserve(this->positions.size());

    for (size_t i = 0; i < this->positions.size(); i++) {
        const auto& pos0 = this->positions[i];
        if (pos0.handles.handle) {
            const auto& pos1 = pos0.handles;

            vertex.push_back(pos0.position);
            vertex.push_back(pos1.handlePosition);
        } else {
            vertex.push_back(pos0.position);
        }
    }
    return vertex;
};

Vec2 DrawPen::shape_data() {
    // NOTE:gets the position of the mesh, important as this will
    // decide if a pen render is correct in sdf or not
    Vec2 position = Vec2(FLT_MAX, FLT_MAX);
    for (const auto& verts : generate_vertices()) {
        position.x = std::min<float>(position.x, verts.x);
        position.y = std::min<float>(position.y, verts.y);
    }
    this->position = position;

    return this->position;
};

int DrawPen::shape_type() const { return static_cast<int>(ShapeType::Pen); };
*/
void Art::Draw() {
    // load shared memory
    Shared::Memory::load_shared_memory();
    {
        // check size first
        ShapeRegistry::check_instance_size();

        const auto shape_size = ShapeRegistry::get_size();

        for (size_t i = 0; i < shape_size; i++) {
            const auto& shape      = ShapeRegistry::instances[i];
            const auto  shape_type = shape.shape_type;

            PushConstants constants{};
            constants.bg_color   = convert_color("#FFFFFF", 1.0f);
            constants.color      = shape.color;
            constants.pos        = shape.position;
            constants.center     = shape.center;
            constants.shape_data = shape.shape_data;
            constants.mesh_size  = shape_mesh(shape_type, i);
            // triangle uses these in shaders instead of shape data
            constants.p0 = shape.p0;
            constants.p1 = shape.p1;
            constants.p2 = shape.p2;
            // for triangle
            constants.stroke     = shape.stroke;
            constants.rotate     = shape.rotate;
            constants.fill       = shape.fill;
            constants.skew       = shape.skew;
            constants.shape_type = shape_type;

            Shared::Memory::register_constants(constants);
            // TODO:this passes the entire size of std::array
            // add size to only pass what the users passed
            // register pen instances
            Shared::Memory::register_pen_data({{{}}});

            // TODO:skew should also work for pen, curently skew mesh is using member
            // "position" and not "positions" which pen uses
            const auto& skew_mesh = get_skew_mesh(constants.mesh_size, constants.pos);

            // TODO:add correct skew pos
            SkewData skew_data{};
            skew_data.skew_mesh = skew_mesh;
            skew_data.skew_pos  = shape.shape_skew.skew_data;

            Shared::Memory::register_skew_data(skew_data);

            // increment instance size, mutates instance size per loop execution
            Shared::Memory::increment_instance_size();
        }
        // reset registry after transfering data to shared memory
        ShapeRegistry::reset_registry();
    }
};

// draw every shape instance registered
/*void Artt::Draw() {
    // load shared memory
    Shared::Memory::load_shared_memory();
    {
        const auto reg_size = InstanceRegistry::get_size();

        for (size_t i = 0; i < reg_size; i++) {
            const auto& inst = InstanceRegistry::get_instance(i);
            // NOTE:execute shape data function first as pen instance heavily depends on
            // it produce a new position using the its generate shape_vertices function
            const auto& shape_data = inst->shape_data();

            PushConstants constants{};
            constants.bg_color   = convert_color(Artt::backgroundColor, 1.0f);
            constants.color      = convert_color(inst->color, inst->opacity);
            constants.pos        = inst->position;
            constants.center     = inst->get_center();
            constants.shape_data = shape_data;
            constants.mesh_size  = mesh_size(inst->generate_vertices());
            constants.p0         = inst->v0;
            constants.p1         = inst->v1;
            constants.p2         = inst->v2;
            constants.stroke     = inst->stroke;
            constants.rotate     = inst->rotate;
            constants.fill       = static_cast<int>(inst->fill);
            constants.skew       = static_cast<int>(inst->skew);
            constants.shape_type = inst->shape_type();

            Shared::Memory::register_constants(constants);
            // register pen instances
            Shared::Memory::register_pen_data(inst->positions);

            // TODO:skew should also work for pen, curently skew mesh is using member
            // "position" and not "positions" which pen uses
            const auto& skew_mesh = get_skew_mesh(constants.mesh_size, inst->position);

            SkewData skew_data{};
            skew_data.skew_mesh = skew_mesh;
            skew_data.skew_pos  = inst->skewPos;

            Shared::Memory::register_skew_data(skew_data);

            // increment instance size, mutates instance size per loop execution
            Shared::Memory::increment_instance_size();
        }
        // reset registry after transfering data to shared memory
        InstanceRegistry::reset_registry();
    }
};*/
