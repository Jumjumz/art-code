#include "artcode.hpp"
#include "artcode_instance.hpp"

// num of times derived class is initialize
static inline int init_count = 0;

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
    Shared::Memory::load_shared_memory();
    // register instance
    Shared::Memory::register_instance(generate_vertices(), generate_indices());
};

DrawQuad::~Quad(){};

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

    Shared::Memory::load_shared_memory();
    // register instance
    Shared::Memory::register_instance(generate_vertices(), generate_indices());
};

DrawCircle::~Circle(){};

ArrayVec2 DrawCircle::generate_vertices() {
    // TODO:add proper vert position for circle
    return ArrayVec2{this->position, this->position + Vec2{0.0f, 0.0f},
                     this->position + Vec2{0.0f, 0.0f}, this->position + Vec2{0.0f, 0.0f}};
};

// TODO:update to correct index connection
ArrayU32 DrawCircle::generate_indices() { return ArrayU32{0, 1, 2, 0, 2, 3}; };

void Art::Draw() {};
