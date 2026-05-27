#include "artcode.hpp"

#include <algorithm>
#include <cassert>
// num of times derived class is initialize
static inline int init_count = 0;

// TODO:backend change, code generation will be removed
struct ShapeRegistry {
    static void register_shape(detail::IPen* shape) {
        if (active_instances.size() == 0)
            active_instances.reserve(20);

        active_instances.push_back(shape);
    }

    static std::vector<detail::IPen*> get_classes() { return active_instances; }

    static void delete_registry(detail::IPen* shape) {
        active_instances.erase(
            std::remove(active_instances.begin(), active_instances.end(), shape),
            active_instances.end());
    }

  private:
    static inline std::vector<detail::IPen*> active_instances;
};

//  instances
using DrawQuad   = Art::Quad;
using DrawCircle = Art::Circle;

// Quad
DrawQuad::Quad() {
    // initialize at object creation
    init_count++;
    this->l        = 100.0f;
    this->w        = 100.0f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    // allocate to the registry
    ShapeRegistry::register_shape(this);
};

DrawQuad::~Quad() { ShapeRegistry::delete_registry(this); };

// Circle
DrawCircle::Circle() {
    // initialize at object creation
    init_count++;
    this->radius   = 0.5f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    // allocate to the registry
    ShapeRegistry::register_shape(this);
};

DrawCircle::~Circle() { ShapeRegistry::delete_registry(this); };

void Art::Draw() {};
