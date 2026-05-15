#include "artcode.hpp"
#include "json.hpp"
#include <cassert>
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
        if (!read.is_open())
            assert("Artcode shader is missing or deleted!");
        while (std::getline(read, line)) {
            lines.push_back(line);
        }
    };
    int line_idx = Circle::SHADER_DECLARATIONS_IDX + Circle::init_count;
    // only write to shader if there is actual changes
    if (lines[line_idx] == glsl_code)
        return;

    Circle::init_lookup[Circle::init_count] = this->name;
    // track the created instance
    track_instances();

    // TODO: add a remove line if sruct instance is deleted/doenst exist
    if (lines[line_idx].find("void main") != string::npos) {
        // insert at fresh creation
        lines.insert(lines.begin() + line_idx, glsl_code);
    } else {
        // replace entire code
        lines.at(line_idx) = glsl_code;
    }

    // write to file
    {
        std::ofstream write(shader_file());
        for (const auto& line : lines) {
            write << line << "\n";
        }
    }
}

void DrawCircle::track_instances() {
    const string   key = "instances";
    nlohmann::json js;
    fs::path       sln_file;
    {
        // search project dir for solution extension
        const auto itr = std::find_if(
            fs::directory_iterator(PROJECT_DIR), fs::directory_iterator{},
            [this](const auto& file) -> bool { return file.path().extension() == ".rcd"; });

        if (itr == fs::directory_iterator()) {
            assert("Solution file is missing or deleted!");
        } else {
            sln_file = PROJECT_DIR / itr->path().filename();
            std::ifstream read(sln_file);
            js = nlohmann::json::parse(read);
        }
    }
    // write into file
    {
        // replace the entire unorderer map entirely regardless of func name or num of instance
        js[key] = Circle::init_lookup;
        std::ofstream write(sln_file);
        write << js.dump(4);
    }
};

void DrawCircle::draw() { write_shader(to_glsl()); };
