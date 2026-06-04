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
    this->l        = 100.0f;
    this->w        = 100.0f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    ShapeRegistry::register_shape(this);
};

DrawQuad::~Quad() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawQuad::generate_vertices() {
    //  quad coordinates and size
    return ArrayVec2{this->position, this->position + Vec2{this->w, 0.0f},
                     this->position + Vec2{this->w, this->l},
                     this->position + Vec2{0.0f, this->l}};
};

ArrayU32 DrawQuad::generate_indices() { return ArrayU32{0, 1, 1, 2, 2, 3, 0, 3}; };

// Circle
DrawCircle::Circle() {
    this->radius   = 100.f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    ShapeRegistry::register_shape(this);
};

DrawCircle::~Circle() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawCircle::generate_vertices() {
    ArrayVec2 vertex;

    for (int i = 0; i < DrawCircle::SEGMENTS; i++) {
        float angle = i * 2.0f * M_PI / DrawCircle::SEGMENTS;

        vertex.push_back(Vec2{this->position.x + cos(angle) * this->radius,
                              this->position.y + sin(angle) * this->radius});
    }
    return vertex;
};

ArrayU32 DrawCircle::generate_indices() {
    ArrayU32 indices;

    for (int i = 0; i < DrawCircle::SEGMENTS; i++) {
        indices.push_back(i);
        indices.push_back((i + 1) % DrawCircle::SEGMENTS);
    }
    return indices;
};

// Triangle
DrawTriangle::Triangle() {
    this->size     = 100.0f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    ShapeRegistry::register_shape(this);
};

DrawTriangle::~Triangle() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawTriangle::generate_vertices() {
    ArrayVec2 vertex;

    for (int i = 0; i < 3; i++) {
        float angle = i * 2.0f * M_PI / 3.0f - M_PI / 2.0f;
        vertex.push_back(Vec2{this->position.x + cos(angle) * this->size,
                              this->position.y + sin(angle) * this->size});
    }
    return vertex;
};

ArrayU32 DrawTriangle::generate_indices() { return ArrayU32{0, 1, 2, 1, 2, 0}; };

void Art::Draw() {
    // load shared memory
    Shared::Memory::load_shared_memory();
    {
        const auto& instances = ShapeRegistry::get_instances();

        for (const auto& inst : instances) {
            // register vert and idx per instance
            Shared::Memory::register_instance(inst->generate_vertices(),
                                              inst->generate_indices());
        }
    }
};
