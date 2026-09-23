#include "artcode.hpp"
#include "artcode_instance.hpp"
#include <cassert>
#include <cmath>
#include <glm/common.hpp>

struct InstanceRegistry {
    static void register_shape(detail::IPen* shape) {
        if (array_size > 500) {
            assert("Max instances of shape registered exceeded");
            return;
        }

        instances[array_size] = shape;
        array_size++;
    }

    static size_t get_size() { return array_size; }

    static detail::IPen* get_instance(size_t index) { return instances[index]; }

    static void reset_registry() {
        array_size = 0;
        std::fill(instances.begin(), instances.end(), nullptr);
    }

  private:
    // init elements to nullptr, max num of instance only allowed
    static inline ArrayT<detail::IPen*, 500> instances  = {};
    static inline size_t                     array_size = 0;
};

enum class ShapeType { Quad, Circle, Triangle, Pen };

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
Vec2 skew_mesh_size(const ArrayVec4& vertices) {
    float max_x = -FLT_MAX, min_x = FLT_MAX;
    float max_y = -FLT_MAX, min_y = FLT_MAX;

    for (const auto& vertex : vertices) {
        max_x = glm::max(max_x, vertex.x);
        min_x = glm::min(min_x, vertex.x);
        max_y = glm::max(max_y, vertex.y);
        min_y = glm::min(min_y, vertex.y);
    }

    return Vec2{max_x - min_x, max_y - min_y};
}

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

ArrayT<Vec2, 36> bezier_curve(const Vec2& handle, const Vec2& st_vec, const Vec2& en_vec) {
    constexpr size_t        LERP_SIZE = 36;
    ArrayT<Vec2, LERP_SIZE> lerp      = {};

    // generate lerp along bezier curve
    for (size_t i = 0; i < LERP_SIZE; i++) {
        float t   = i / static_cast<float>(LERP_SIZE - 1);
        float t2  = squared(t);
        float mt  = 1.0f - t;
        float mt2 = squared(mt);

        // quardratic bezier formula
        const auto pt = Vec2{(mt2 * st_vec.x) + (2 * mt * t * handle.x) + (t2 * en_vec.x),
                             (mt2 * st_vec.y) + (2 * mt * t * handle.y) + (t2 * en_vec.y)};

        lerp[i] = pt;
    }
    return lerp;
};

// TODO:remove vertices and indices functions
// API implementations
using DrawQuad     = Art::Quad;
using DrawCircle   = Art::Circle;
using DrawTriangle = Art::Triangle;
using DrawPen      = Art::Pen;

// Quad
DrawQuad::Quad()
    : l(100),
      w(100) {
    InstanceRegistry::register_shape(this);
};

// NOTE:all generate vertices function for shapes except Pen has 10.0f for w and z
// properties this to isolate real usage of w and z in Pen tool
// will update in the future
ArrayVec4 DrawQuad::generate_vertices() const {
    //  quad coordinates and size
    return ArrayVec4{
        Vec4{this->position, Vec2{2.0f, 2.0f}},
        Vec4{this->position + Vec2{this->w, 0.0f}, Vec2{2.0f, 2.0f}},
        Vec4{this->position + Vec2{this->w, this->l}, Vec2{2.0f, 2.0f}},
        Vec4{this->position + Vec2{0.0f, this->l}, Vec2{2.0f, 2.0f}},
    };
};

ArrayU32 DrawQuad::generate_indices() const { return ArrayU32{0, 1, 3, 1, 2, 3}; };

Vec2 DrawQuad::shape_data() { return Vec2{this->w, this->l}; };

int DrawQuad::shape_type() const { return static_cast<int>(ShapeType::Quad); };

// Circle
DrawCircle::Circle()
    : radius(100.0f) {
    InstanceRegistry::register_shape(this);
};

ArrayVec4 DrawCircle::generate_vertices() const {
    ArrayVec4 vertex;
    // center of the circle
    vertex.push_back({this->position, Vec2{10.0f, 10.0f}});

    const size_t num_seg = 8;
    for (size_t i = 0; i < num_seg; i++) {
        float angle = i * 2.0f * M_PI / num_seg;

        vertex.push_back(Vec4{this->position.x + cos(angle) * this->radius,
                              this->position.y + sin(angle) * this->radius, 2.0f, 2.0f});
    }
    return vertex;
};

ArrayU32 DrawCircle::generate_indices() const {
    ArrayU32 indices = {};

    const auto num_seg = 8;
    for (size_t i = 0; i < num_seg; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back((i + 1) % num_seg + 1);
    }

    return indices;
};

Vec2 DrawCircle::shape_data() { return Vec2{this->radius, this->radius}; };

int DrawCircle::shape_type() const { return static_cast<int>(ShapeType::Circle); };

// Triangle
DrawTriangle::Triangle() { InstanceRegistry::register_shape(this); };

ArrayVec4 DrawTriangle::generate_vertices() const {
    return ArrayVec4{Vec4{this->v0, Vec2{2.0f, 2.0f}}, Vec4{this->v1, Vec2{2.0f, 2.0f}},
                     Vec4{this->v2, Vec2{2.0f, 2.0f}}};
};

ArrayU32 DrawTriangle::generate_indices() const { return ArrayU32{0, 1, 2}; };

// NOTE:triangle doesnt need to return shape data
// just needs to assign p0, p1 and p2
Vec2 DrawTriangle::shape_data() {
    this->v0 += this->position;
    this->v1 += this->position;
    this->v2 += this->position;

    return this->v0;
};

int DrawTriangle::shape_type() const { return static_cast<int>(ShapeType::Triangle); };

DrawPen::Pen()
    : positions({}) {
    InstanceRegistry::register_shape(this);
};

// NOTE: currently the only place where w and z is important and is being used in the shader
ArrayVec4 DrawPen::generate_vertices() const {
    ArrayVec4 vertex = {};
    vertex.reserve(this->positions.size());

    for (size_t i = 0; i < this->positions.size(); i++) {
        const auto& pos0 = this->positions[i];
        if (pos0.handles.handle) {
            const auto& pos1 = pos0.handles;
            const auto& pos2 = this->positions[i + 1];

            vertex.push_back(Vec4{pos0.position, Vec2{0.0f, 0.0f}});
            vertex.push_back(Vec4{pos1.handlePosition, Vec2{0.0f, 0.5f}});
        } else {
            vertex.push_back(Vec4{pos0.position, Vec2{1.0f, 1.0f}});
        }
    }
    return vertex;
};

// NOTE:this generates duplicate indices for index 0
ArrayU32 DrawPen::generate_indices() const {
    ArrayU32 indices = {};

    const auto pen_size = generate_vertices().size();
    for (size_t i = 0; i < pen_size - 1; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    return indices;
};

// TODO:add v0, v1 and v2 for line curves
Vec2 DrawPen::shape_data() {
    // FIXME:this will not work for multiple curves.. should have a different approach
    // NOTE:mutating v0, v1 and v2 doesnt have to do with shape data,
    // this is for test purpose
    for (size_t i = 0; i < this->positions.size(); i++) {
        const auto& pos = this->positions[i];
        if (pos.handles.handle) {
            this->v0 = pos.position;
            this->v1 = pos.handles.handlePosition;
            this->v2 = this->positions[i + 1].position;
            continue;
        }
    }

    return this->position;
};

int DrawPen::shape_type() const { return static_cast<int>(ShapeType::Pen); };

// draw every shape instance registered
void Art::Draw() {
    // load shared memory
    Shared::Memory::load_shared_memory();
    {
        const auto& reg_size = InstanceRegistry::get_size();
        for (size_t i = 0; i < reg_size; i++) {
            const auto& inst = InstanceRegistry::get_instance(i);

            PushConstants constants{};
            constants.bg_color   = convert_color(Art::backgroundColor, 1.0f);
            constants.color      = convert_color(inst->color, inst->opacity);
            constants.pos        = inst->position;
            constants.center     = inst->get_center();
            constants.shape_data = inst->shape_data();
            constants.mesh_size  = skew_mesh_size(inst->generate_vertices());
            constants.p0         = inst->v0;
            constants.p1         = inst->v1;
            constants.p2         = inst->v2;
            constants.stroke     = inst->stroke;
            constants.rotate     = inst->rotate;
            constants.fill       = static_cast<int>(inst->fill);
            constants.skew       = static_cast<int>(inst->skew);
            constants.shape_type = inst->shape_type();

            // TODO:skew should also work for pen, curently skew mesh is using member
            // "position" and not "positions" which pen uses
            const auto& skew_mesh = get_skew_mesh(constants.mesh_size, inst->position);

            SkewData skew_data;
            skew_data.skew_mesh = skew_mesh;
            skew_data.skew_pos  = inst->skewPos;
            //  register pen instances
            if (constants.shape_type == static_cast<int>(ShapeType::Pen)) {
                // NOTE:pen instance is not responsible for increasing the shared memory size,
                //  that is register_constants responsibility
                Shared::Memory::register_pen_instance(inst->generate_vertices(),
                                                      inst->generate_indices());
            }

            Shared::Memory::register_constants(constants, skew_data);
        }
    }
};
