#include "artcode.hpp"
#include "artcode_instance.hpp"
#include <cmath>
#include <glm/common.hpp>

struct ShapeRegistry {
    static void register_shape(detail::IPen* shape) {
        if (instances.size() == 0)
            instances.reserve(20);

        instances.push_back(shape);
    }

    static VectorT<detail::IPen*> get_instances() { return instances; }

    static void delete_registry(detail::IPen* shape) {
        instances.erase(std::remove(instances.begin(), instances.end(), shape),
                        instances.end());
    }

  private:
    // TODO:find a way where instances throws a compiler error if num of instance exceeds
    // 500 to avoid error in shared memory
    static inline VectorT<detail::IPen*> instances;
};

Vec4 convert_color(const string& color, float opacity) {
    string hex   = color[0] == '#' ? color.substr(1) : color;
    u32    value = std::stoul(hex, nullptr, 16);

    // 0xFF (255) is a bit mask
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
Vec2 skew_mesh_size(const ArrayVec2& vertices, const Vec2& center) {
    Vec2 len_width = Vec2{0.0f, 0.0f};
    Vec2 v_x       = Vec2{0.0f, 0.0f};
    Vec2 v_y       = Vec2{0.0f, 0.0f};
    for (const auto& vertex : vertices) {
        // get min and max
        float max_x = glm::max(vertex.x - center.x, vertex.x);
        float min_x = glm::min(vertex.x - center.x, vertex.x);
        float max_y = glm::max(vertex.y - center.y, vertex.y);
        float min_y = glm::min(vertex.y - center.y, vertex.y);
        // reduction operation
        if (v_x.x > max_x) {
            v_x.x = max_x;
        }
        if (v_x.y < min_x) {
            v_x.y = min_x;
        }
        if (v_y.x > max_y) {
            v_y.x = max_y;
        }
        if (v_y.y < min_y) {
            v_y.y = min_y;
        }
    }
    // calculate the max minus min
    len_width.x = v_x.x - (v_x.y * -1) * 2;
    len_width.y = v_y.x - (v_y.y * -1) * 2;

    return len_width;
};

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

ArrayVec2 bezier_curve(const Vec2& handle, const Vec2& st_vec, const Vec2& en_vec) {
    constexpr size_t SEG = 8;

    ArrayVec2 lerp;
    // generate lerp along bezier curve
    for (size_t i = 0; i <= SEG; i++) {
        float t   = i / (float)SEG;
        float t2  = squared(t);
        float mt  = 1.0f - t;
        float mt2 = squared(mt);

        // quardratic bezier formula
        const auto& pt =
            Vec2{(mt2 * st_vec.x) + (2 * mt * t * handle.x) + (t2 * en_vec.x),
                 (mt2 * st_vec.y) + (2 * mt * t * handle.y) + (t2 * en_vec.y)};

        lerp.push_back(pt);
    }
    return lerp;
};

using DrawQuad     = Art::Quad;
using DrawCircle   = Art::Circle;
using DrawTriangle = Art::Triangle;
using DrawPen      = Art::Pen;

// Quad
DrawQuad::Quad() {
    this->l        = 100.0f;
    this->w        = 100.0f;
    this->position = Vec2{200, 200};
    this->color    = "#000000";
    this->stroke   = 1.0f;
    this->rotate   = 0.0f;
    this->opacity  = 1.0f;
    this->fill     = false;
    this->skew     = false;
    this->skewPos  = {};

    ShapeRegistry::register_shape(this);
};

DrawQuad::~Quad() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawQuad::generate_vertices() const {
    //  quad coordinates and size
    return ArrayVec2{
        this->position,
        this->position + Vec2{this->w, 0.0f},
        this->position + Vec2{this->w, this->l},
        this->position + Vec2{0.0f, this->l},
    };
};

ArrayU32 DrawQuad::generate_indices() const {
    if (this->fill) {
        // triangles
        return ArrayU32{0, 1, 3, 1, 2, 3};
    } else {
        // lines
        return ArrayU32{0, 1, 1, 2, 2, 3, 3, 0};
    }
};

VectorT<Handles> DrawQuad::generate_handles() {
    // TODO:this will be used for warp in the future
    return VectorT<Handles>{{Vec2{0, 0}, static_cast<int>(false)}};
};

// Circle
DrawCircle::Circle() {
    this->radius   = 100.f;
    this->position = Vec2{200, 200};
    this->color    = "#000000";
    this->stroke   = 1.0f;
    this->rotate   = 0.0f;
    this->opacity  = 1.0f;
    this->fill     = false;
    this->skew     = false;
    this->skewPos  = {};

    ShapeRegistry::register_shape(this);
};

DrawCircle::~Circle() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawCircle::generate_vertices() const {
    ArrayVec2 vertex;
    // center of the circle
    vertex.push_back(this->position);

    for (int i = 0; i < DrawCircle::SEGMENTS; i++) {
        float angle = i * 2.0f * M_PI / DrawCircle::SEGMENTS;

        vertex.push_back(Vec2{this->position.x + cos(angle) * this->radius,
                              this->position.y + sin(angle) * this->radius});
    }
    return vertex;
};

ArrayU32 DrawCircle::generate_indices() const {
    ArrayU32 indices;

    if (this->fill) {
        // triangles
        for (int i = 0; i < DrawCircle::SEGMENTS; i++) {
            indices.push_back(0);
            indices.push_back(i + 1);
            indices.push_back((i + 1) % DrawCircle::SEGMENTS + 1);
        }
    } else {
        // lines
        for (int i = 0; i < DrawCircle::SEGMENTS; i++) {
            indices.push_back(i + 1);
            indices.push_back((i + 1) % DrawCircle::SEGMENTS + 1);
        }
    }
    return indices;
};

VectorT<Handles> DrawCircle::generate_handles() {
    return VectorT<Handles>{{Vec2{0, 0}, static_cast<int>(false)}};
};

// TODO:have a way where compiler identifies the type first, then from there triangle
// can only provide if user has access to base and height or only base if type is
// equilateral
// Triangle
DrawTriangle::Triangle() {
    this->base     = 100.0f;
    this->height   = 100.0f;
    this->type     = TriangleTypes::Equilateral;
    this->position = Vec2{200, 200};
    this->color    = "#000000";
    this->stroke   = 1.0f;
    this->rotate   = 0.0f;
    this->opacity  = 1.0f;
    this->fill     = false;
    this->skew     = false;
    this->skewPos  = {};

    ShapeRegistry::register_shape(this);
};

DrawTriangle::~Triangle() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawTriangle::generate_vertices() const {
    ArrayVec2 vertex;

    switch (this->type) {
    case TriangleTypes::Equilateral: {
        float size = this->base * glm::sqrt(3) / 2;
        for (int i = 0; i < 3; i++) {
            float angle = i * 2.0f * M_PI / 3.0f - M_PI / 2.0f;
            vertex.push_back(Vec2{this->position.x + cos(angle) * size,
                                  this->position.y + sin(angle) * size});
        }
        break;
    }
    case TriangleTypes::Right: {
        vertex = ArrayVec2{this->position, this->position + Vec2{this->base, 0.0f},
                           this->position - Vec2{0.0f, this->height}};
        break;
    }
    }
    return vertex;
};

ArrayU32 DrawTriangle::generate_indices() const {
    if (this->fill) {
        return ArrayU32{0, 1, 2};
    } else {
        // line list
        return ArrayU32{0, 1, 1, 2, 2, 0};
    }
};

VectorT<Handles> DrawTriangle::generate_handles() {
    return VectorT<Handles>{{Vec2{0, 0}, static_cast<int>(false)}};
};

DrawPen::Pen() {
    this->positions = {};
    this->position  = Vec2{200, 200};
    this->color     = "#000000";
    this->stroke    = 1.0f;
    this->rotate    = 0.0f;
    this->opacity   = 1.0f;
    this->fill      = false;
    this->skew      = false;
    this->skewPos   = {};

    ShapeRegistry::register_shape(this);
};

DrawPen::~Pen() { ShapeRegistry::delete_registry(this); };

// TODO:add the bezier curve function to generate vertices to pass the entire vertices all
// in one buffer with the curves
ArrayVec2 DrawPen::generate_vertices() const {
    ArrayVec2 vertex;
    for (size_t i = 0; i < this->positions.size(); i++) {
        const auto& pos = this->positions[i];
        if (pos.handles.handle == 1) {
            const auto& pos2 = this->positions[i + 1];
            const auto& bezier =
                bezier_curve(pos.handles.handlePosition, pos.position, pos2.position);

            // flatten the bezeir array
            vertex.push_back(pos.position);
            for (const auto& bez : bezier) {
                vertex.push_back(bez);
            }
        } else {
            vertex.push_back(pos.position);
        }
    }
    return vertex;
};

ArrayU32 DrawPen::generate_indices() const {
    ArrayU32   indices;
    const auto pos_size = generate_vertices().size();
    if (this->fill) {
        for (size_t i = 0; i < pos_size - 1; i++) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    } else {
        for (size_t i = 0; i < pos_size - 1; i++) {
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }

    return indices;
};

// TODO:remove generate handles and the entire config, it is not needed as bezeir curve is passed from CPU anyways
VectorT<Handles> DrawPen::generate_handles() {
    this->position = this->positions[0].position;

    VectorT<Handles> handle_data = {};
    for (auto& position : this->positions) {
        auto& handles = position.handles;
        // static cast stays even though the current type of handle is an int
        handles.handle = static_cast<int>(handles.handle);
        handle_data.push_back(handles);
    }
    return handle_data;
};

// draw every shape instance registered
void Art::Draw() {
    // load shared memory
    Shared::Memory::load_shared_memory();
    // reset/clear previous instances
    Shared::Memory::reset_instance();
    {
        const auto& instances = ShapeRegistry::get_instances();
        for (const auto& inst : instances) {
            PushConstants constants;
            constants.color  = convert_color(inst->color, inst->opacity);
            constants.center = inst->get_center();
            constants.stroke = inst->stroke;
            constants.rotate = inst->rotate;
            constants.fill   = static_cast<int>(inst->fill);
            constants.skew   = static_cast<int>(inst->skew);

            // TODO:skew should also work for pen, curently skew mesh is using member
            // "position" and not "positions" which pen uses
            const auto& skew_mesh =
                get_skew_mesh(skew_mesh_size(inst->generate_vertices(), inst->get_center()),
                              inst->position);

            SkewData skew_data;
            skew_data.skew_mesh = skew_mesh;
            skew_data.skew_pos  = inst->skewPos;

            // register resources per instance
            Shared::Memory::register_instance(inst->generate_vertices(),
                                              inst->generate_indices(), constants,
                                              skew_data, inst->generate_handles());
        }
    }
};
