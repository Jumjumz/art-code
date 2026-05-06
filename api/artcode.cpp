#include "artcode.hpp"

using DrawCircle = Art::Circle;
// Circle
DrawCircle::Circle() {
    // initialize at object creation
    this->name = "circle";
    this->radius = 0.5f;
    this->position = Vec2{200, 200};
    this->color = Vec3{0.0f, 0.0f, 0.0f};
    this->stroke = 1.0f;
    this->scale = 1.0f;
};

string DrawCircle::to_glsl() const {
    string glsl_code = "float " + this->name + "()" + "{";
    glsl_code += "return length(artboard_pos - vec2(" +
                 std::to_string(this->position.x) + "," +
                 std::to_string(this->position.y) + ")" + ") - " +
                 std::to_string(this->radius) + ";";
    glsl_code += "}";

    return glsl_code;
};

void DrawCircle::write_shader(const string &glsl_code) {};

void DrawCircle::draw() { write_shader(to_glsl()); };
