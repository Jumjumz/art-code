#include "artcode.hpp"
#include "artcode_instance.hpp"
#include <cmath>

struct ShapeRegistry {
    static void register_shape(detail::IPen* shape) {
        if (instances.size() == 0)
            instances.reserve(20);

        instances.push_back(shape);
    }

    static std::vector<detail::IPen*> get_instances() { return instances; }

    static void delete_registry(detail::IPen* shape) {
        instances.erase(std::remove(instances.begin(), instances.end(), shape),
                        instances.end());
    }

  private:
    static inline std::vector<detail::IPen*> instances;
};

using DrawQuad     = Art::Quad;
using DrawCircle   = Art::Circle;
using DrawTriangle = Art::Triangle;

// Quad
DrawQuad::Quad() {
    this->l            = 100.0f;
    this->w            = 100.0f;
    this->position     = Vec2{200, 200};
    this->skewPosition = Vec2{0.0f, 0.0f};
    this->color        = "#000000";
    this->stroke       = 1.0f;
    this->rotate       = 0.0f;
    this->opacity      = 1.0f;
    this->fill         = false;
    this->skew         = false;
    this->skewIndex    = 0;

    ShapeRegistry::register_shape(this);
};

DrawQuad::~Quad() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawQuad::generate_vertices() const {
    //  quad coordinates and size
    return ArrayVec2{this->position,
                     this->position + Vec2{this->w * 0.5f, 0.0f},
                     this->position + Vec2{this->w, 0.0f},
                     this->position + Vec2{this->w, this->l * 0.5f},
                     this->position + Vec2{this->w, this->l},
                     this->position + Vec2{this->w * 0.5f, this->l},
                     this->position + Vec2{0.0f, this->l},
                     this->position + Vec2{0.0f, this->l * 0.5f}};
};

ArrayU32 DrawQuad::generate_indices() const {
    if (this->fill) {
        // triangles
        return ArrayU32{0, 2, 6, 2, 4, 6};
    } else {
        return ArrayU32{0, 2, 2, 4, 4, 6, 6, 0};
    }
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

    ShapeRegistry::register_shape(this);
};

DrawCircle::~Circle() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawCircle::generate_vertices() const {
    ArrayVec2 vertex;

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
            indices.push_back(i);
            indices.push_back((i + 1) % DrawCircle::SEGMENTS);
        }
    } else {
        for (int i = 0; i < DrawCircle::SEGMENTS; i++) {
            indices.push_back(i);
            indices.push_back((i + 1) % DrawCircle::SEGMENTS);
        }
    }
    return indices;
};

// Triangle
DrawTriangle::Triangle() {
    this->size     = 100.0f;
    this->type     = TriangleTypes::Equilateral;
    this->position = Vec2{200, 200};
    this->color    = "#000000";
    this->stroke   = 1.0f;
    this->rotate   = 0.0f;
    this->opacity  = 1.0f;
    this->fill     = false;

    ShapeRegistry::register_shape(this);
};

DrawTriangle::~Triangle() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawTriangle::generate_vertices() const {
    ArrayVec2 vertex;

    switch (this->type) {
    case TriangleTypes::Equilateral: {
        for (int i = 0; i < 3; i++) {
            float angle = i * 2.0f * M_PI / 3.0f - M_PI / 2.0f;
            vertex.push_back(Vec2{this->position.x + cos(angle) * this->size,
                                  this->position.y + sin(angle) * this->size});
        }
        break;
    }
    case TriangleTypes::Right: {
        vertex = ArrayVec2{this->position, this->position + Vec2{this->size, 0.0f},
                           this->position - Vec2{0.0f, this->size}};
        break;
    }
    }
    return vertex;
};

ArrayU32 DrawTriangle::generate_indices() const { return ArrayU32{0, 1, 2}; };

void Art::Draw() {
    // load shared memory
    Shared::Memory::load_shared_memory();
    {
        const auto& instances = ShapeRegistry::get_instances();
        // TODO:skew only works for quad, should also work for other shapes
        for (const auto& inst : instances) {
            Vec2 vec_idx = inst->generate_vertices()[inst->skewIndex];
            // register vert and idx per instance
            Shared::Memory::register_instance(inst->generate_vertices(),
                                              inst->generate_indices(),
                                              {.color     = inst->convert_color(),
                                               .center    = inst->get_center(),
                                               .skew_pos  = inst->skewPosition,
                                               .skew_vert = vec_idx,
                                               .stroke    = inst->stroke,
                                               .rotate    = inst->rotate,
                                               .fill      = static_cast<int>(inst->fill),
                                               .skew      = static_cast<int>(inst->skew),
                                               .skew_idx  = inst->skewIndex});
        }
    }
};
