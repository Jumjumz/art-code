#include "artcode.hpp"
#include "json.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>

// filesystem
namespace fs = std::filesystem;

// static variables
const fs::path PROJECT_DIR = fs::canonical("/proc/self/exe").parent_path().parent_path();

// shader lines of init, idx that contains version and layout keywords 0 -> 3
static constexpr int SHADER_DECLARATIONS_IDX = 3;

static inline UMap init_lookup;
// num of times derived class is initialize
static inline int init_count = 0;

fs::path shader_file() { return PROJECT_DIR / "shaders" / "artcode.frag"; };

using NJson = nlohmann::json;

struct ShapeRegistry {
    static void register_shape(Art::detail::IPen* shape) {
        active_classes.push_back(shape);
    }

    static std::vector<Art::detail::IPen*> get_classes() { return active_classes; }

  private:
    static inline std::vector<Art::detail::IPen*> active_classes;
};

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

void write_shader(const ArrayString& lines) {
    // write to file
    std::ofstream write(shader_file());
    for (const auto& line : lines) {
        write << line << "\n";
    }
};

ArrayString read_lines() {
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

    return lines;
};

void Art::Draw() {
    auto lines = read_lines();

    const auto active_classes = ShapeRegistry::get_classes();
    // check num of instances
    for (int i = 0; i < active_classes.size(); i++) {
        const auto glsl_code = active_classes[i]->to_glsl();
        // TODO:before writing, check if func name already exist first
        // always +1 for precise insert in line 5 for 1st instance
        const int line_idx = SHADER_DECLARATIONS_IDX + (i + 1);
        // only write to shader if there is actual changes
        if (lines[line_idx] == glsl_code)
            continue;

        // TODO: add a remove line if sruct instance is deleted/doenst exist
        if (lines[line_idx].find("void main") != string::npos) {
            // insert at fresh creation
            lines.insert(lines.begin() + line_idx, glsl_code);
        } else {
            // replace entire code
            lines.at(line_idx) = glsl_code;
        }
    }
    // write in shader all at once
    write_shader(lines);
    /*const auto instances = created_instances();
    for (const auto& [key, val] : instances) {
    }*/
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

    // allocate to the registry
    ShapeRegistry::register_shape(this);
};

string DrawCircle::to_glsl() const {
    string glsl_code = "float " + this->name + "()" + "{";
    glsl_code += "return length(artboard_pos - vec2(" + ToString(this->position.x) + "," +
                 ToString(this->position.y) + ")" + ") - " + // vulkan is y inverse
                 ToString(this->radius) + ";";
    glsl_code += "}";

    return glsl_code;
};
