#include "artcode.hpp"
#include "artcode_instance.hpp"

// num of times derived class is initialize
static inline int init_count = 0;

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
    // TODO:load share memory should not be here, should be somewhere where it only runs
    // once in api and not per instance
    ShapeRegistry::register_shape(this);
    Shared::Memory::load_shared_memory();
};

DrawQuad::~Quad() {
    ShapeRegistry::delete_registry(this);
    // Shared::Memory::reset_instance();
};

ArrayVec2 DrawQuad::generate_vertices() {
    //  quad coordinates and size
    return ArrayVec2{this->position, this->position + Vec2{this->w, 0.0f},
                     this->position + Vec2{this->w, this->l},
                     this->position + Vec2{0.0f, this->l}};
};

ArrayU32 DrawQuad::generate_indices() { return ArrayU32{0, 1, 2, 0, 2, 3}; };

// Circle
DrawCircle::Circle() {
    // initialize at object creation
    init_count++;
    this->radius   = 0.5f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    ShapeRegistry::register_shape(this);
    Shared::Memory::load_shared_memory();
};

DrawCircle::~Circle() { ShapeRegistry::delete_registry(this); };

ArrayVec2 DrawCircle::generate_vertices() {
    // TODO:add proper vert position for circle
    return ArrayVec2{this->position, this->position + Vec2{0.0f, 0.0f},
                     this->position + Vec2{0.0f, 0.0f}, this->position + Vec2{0.0f, 0.0f}};
};

// TODO:update to correct index connection
ArrayU32 DrawCircle::generate_indices() { return ArrayU32{0, 1, 2, 0, 2, 3}; };

void Art::Draw() {
    const auto& instances = ShapeRegistry::get_instances();

    for (const auto& inst : instances) {
        // register vert and idx per instance
        Shared::Memory::register_instance(inst->generate_vertices(),
                                          inst->generate_indices());
    }
};
