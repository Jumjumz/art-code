#include "artcode.hpp"
#include "artcode_instance.hpp"

#include <cassert>
#include <iostream>
// num of times derived class is initialize
static inline int init_count = 0;

// TODO:backend change, code generation will be removed
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

    // register instance
    ArtcodeInstance::register_instance(this);
};

DrawQuad::~Quad() { ArtcodeInstance::delete_instance(this); };

ArrayVec2 DrawQuad::generate_vertices() {
    //  quad coordinates and size
    return {this->position, this->position + Vec2{this->w, 0.0f},
            this->position + Vec2{this->w, this->l}, this->position + Vec2{0.0f, this->l}};
};

ArrayU32 DrawQuad::generate_indices() { return {0, 1, 2, 0, 2, 3}; };

// Circle
DrawCircle::Circle() {
    // initialize at object creation
    init_count++;
    this->radius   = 0.5f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    // register instance
    ArtcodeInstance::register_instance(this);
};

DrawCircle::~Circle() { ArtcodeInstance::delete_instance(this); };

ArrayVec2 DrawCircle::generate_vertices() {
    // TODO:add proper vert position for circle
    return {this->position, this->position + Vec2{0.0f, 0.0f},
            this->position + Vec2{0.0f, 0.0f}, this->position + Vec2{0.0f, 0.0f}};
};

// TODO:update to correct index connection
ArrayU32 DrawCircle::generate_indices() { return {0, 1, 2, 0, 2, 3}; };

void Art::Draw() {
    for (const auto instance : ArtcodeInstance::get_instance()) {
        std::cout << &instance << std::endl;

        instance->generate_vertices();
        instance->generate_indices();
    }
};
