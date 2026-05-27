#include "artcode.hpp"

#include <algorithm>
#include <cassert>
// num of times derived class is initialize
static inline int init_count = 0;

// TODO:backend change, code generation will be removed
struct ShapeRegistry {
    static void register_shape(detail::IPen* shape) {
        if (active_classes.size() == 0)
            active_classes.reserve(20);

        active_classes.push_back(shape);
    }

    static std::vector<detail::IPen*> get_classes() { return active_classes; }

    static void delete_registry(detail::IPen* shape) {
        active_classes.erase(
            std::remove(active_classes.begin(), active_classes.end(), shape),
            active_classes.end());
    }

  private:
    static inline std::vector<detail::IPen*> active_classes;
};

//  instances
using DrawCircle = Art::Circle;
using DrawQuad   = Art::Quad;

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

void Art::Draw() {};
