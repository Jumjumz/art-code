#include "artcode.hpp"
#include "json.hpp"
#include <cassert>
#include <fstream>

using DrawCircle = Art::Circle;
using NJson      = nlohmann::json;

static struct InstanceTracking {
    const string key = "instances";

    NJson    js;
    fs::path sln_file;
} init_tracking;

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
            assert("Cannot find artcode frag shader, either is missing or deleted!");
        while (std::getline(read, line)) {
            lines.push_back(line);
        }
    };
    int line_idx = Circle::SHADER_DECLARATIONS_IDX + Circle::init_count;
    // only write to shader if there is actual changes
    if (lines[line_idx] == glsl_code)
        return;

    Circle::init_lookup[Circle::init_count] = this->name;

    // TODO:check instances if it matches the init_lookup
    const auto instances = created_instances();
    if (instances == Circle::init_lookup) {
    }

    // TODO: add a remove line if sruct instance is deleted/doenst exist
    if (lines[line_idx].find("void main") != string::npos) {
        // insert at fresh creation
        lines.insert(lines.begin() + line_idx, glsl_code);
    } else {
        // replace entire code
        lines.at(line_idx) = glsl_code;
    }

    // track the created instance
    track_instances();

    // write to file
    {
        std::ofstream write(shader_file());
        for (const auto& line : lines) {
            write << line << "\n";
        }
    }
};

UMap DrawCircle::created_instances() {
    // search project dir for solution extension
    const auto itr = std::find_if(
        fs::directory_iterator(PROJECT_DIR), fs::directory_iterator{},
        [this](const auto& file) -> bool { return file.path().extension() == ".rcd"; });

    if (itr == fs::directory_iterator()) {
        assert("Cannot find solution file, either not in this project directory or "
               "deleted!");
    } else {
        init_tracking.sln_file = PROJECT_DIR / itr->path().filename();
        std::ifstream read(init_tracking.sln_file);
        init_tracking.js = NJson::parse(read);
    }

    return init_tracking.js[init_tracking.key].get<UMap>();
};

void DrawCircle::track_instances() {
    // replace the entire unorderer map entirely regardless of func name or num of instance
    init_tracking.js[init_tracking.key] = Circle::init_lookup;
    std::ofstream write(init_tracking.sln_file);
    write << init_tracking.js.dump(4);

    write.close();
};

void DrawCircle::draw() { write_shader(to_glsl()); };
