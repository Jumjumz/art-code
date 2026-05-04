#include "artcode.hpp"

using DrawCircle = Art::Circle;
// Circle
DrawCircle::Circle() {
    // initialize at object creation
    this->radius = 0.5f;
    this->position = Vec2{200, 200};
    this->color = Vec3{0.0f, 0.0f, 0.0f};
    this->stroke = 1.0f;
    this->scale = 1.0f;
};

void DrawCircle::draw() {};
