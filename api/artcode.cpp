#include "artcode.hpp"
#include <algorithm>
#include <fstream>

using DrawCircle = Art::Circle;
// Circle
DrawCircle::Circle() {
    // initialize at object creation
    this->name     = "circle";
    this->radius   = 0.5f;
    this->position = Vec2{200, -200};
    this->color    = Vec3{0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;
};

string DrawCircle::to_glsl() const {
    string glsl_code = "float " + this->name + "()" + "{";
    glsl_code += "return length(artboard_pos - vec2(" + ToString(this->position.x) + "," +
                 ToString(this->position.y) + ")" + ") - " + // vulkan is y inverse
                 ToString(this->radius) + ";";
    glsl_code += "}";

    return glsl_code;
};

void DrawCircle::write_shader(const string& glsl_code) {
    ArrayString lines;
    // reserve 100 lines
    lines.reserve(100);
    // read file
    {
        std::ifstream read(shader_file());
        string        line;
        while (std::getline(read, line)) {
            lines.push_back(line);
        }
    }

    // TODO:check for the whole code.. replace code if there are changes
    if (std::find(lines.begin(), lines.end(), glsl_code) == lines.end()) {
        // FIXME:reimplement logic, this is wrong
        if (glsl_code.find(this->name) == string::npos) {
            lines.insert(lines.begin() + 3, glsl_code);
        } else {
            lines.at(3) = glsl_code;
        }
    }

    // write to file
    {
        std::ofstream write(shader_file());
        for (const auto& line : lines) {
            write << line << "\n";
        }
    }
}

void DrawCircle::draw() { write_shader(to_glsl()); };
