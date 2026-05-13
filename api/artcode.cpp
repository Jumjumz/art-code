#include "artcode.hpp"
#include <fstream>

using DrawCircle = Art::Circle;
// Circle
DrawCircle::Circle() {
    // initialize at object creation
    // TODO:init count should be tracked, if instance is deleted so is the code generated
    Circle::init_count++;
    this->name     = "circle_" + ToString(Circle::init_count);
    this->radius   = 0.5f;
    this->position = Vec2{200, 200};
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
    };

    // TODO:should have a check if function name (this->name) has been updated
    int line_idx = Circle::SHADER_DECLARATIONS_IDX + Circle::init_count;
    // check if glsl code is inserted or not
    if (lines[line_idx].find(this->name) == glsl_code.find(this->name)) {
        // replace if updates code is updated
        lines.at(static_cast<size_t>(line_idx)) = glsl_code;
    } else {
        // insert at fresh creation
        lines.insert(lines.begin() + line_idx, glsl_code);
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
