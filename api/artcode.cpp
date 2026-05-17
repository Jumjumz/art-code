#include "artcode.hpp"
#include "json.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>

// filesystem
namespace fs = std::filesystem;

// static variables
const fs::path PROJECT_DIR = fs::canonical("/proc/self/exe").parent_path().parent_path();

fs::path shader_file() { return PROJECT_DIR / "shaders" / "artcode.frag"; };

// shader lines of init, idx that contains version and layout keywords 0 -> 3
static constexpr int SHADER_DECLARATIONS_IDX = 3;

static inline UMap init_lookup;
// num of times derived class is initialize
static inline int init_count = 0;

using NJson = nlohmann::json;

static struct InstanceTracking {
    const string key = "instances";

    NJson    js;
    fs::path sln_file;
} init_tracking;

UMap created_instances() {
    // search project dir for solution extension
    const auto itr = std::find_if(
        fs::directory_iterator(PROJECT_DIR), fs::directory_iterator{},
        [&](const auto& file) -> bool { return file.path().extension() == ".rcd"; });

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

void track_instances() {
    // replace the entire unorderer map entirely regardless of func name or num of instance
    init_tracking.js[init_tracking.key] = init_lookup;
    std::ofstream write(init_tracking.sln_file);
    write << init_tracking.js.dump(4);
};

void write_shader(const string& glsl_code) {
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
    const int line_idx = SHADER_DECLARATIONS_IDX + init_count;
    // only write to shader if there is actual changes
    if (lines[line_idx] == glsl_code)
        return;

    // get function name ie. float circle_1
    const auto func_name    = glsl_code.find("()");
    init_lookup[init_count] = glsl_code.substr(0, func_name);

    // TODO:check instances if it matches the init_lookup
    const auto instances = created_instances();
    {
        const auto it = instances.find(init_count);
        // key must exist and func name must match
        if (it != instances.end() && it->second == "") {
        }
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

using DrawCircle = Art::Circle;

// Circle
DrawCircle::Circle() {
    // initialize at object creation
    // TODO:init count should be tracked, if instance is deleted so is the code generated
    init_count++;
    this->name     = "circle_" + ToString(init_count);
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

void DrawCircle::draw() { write_shader(to_glsl()); };
