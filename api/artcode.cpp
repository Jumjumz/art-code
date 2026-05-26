#include "artcode.hpp"
#include "json.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>

// filesystem
namespace fs = std::filesystem;

// static variables
const fs::path PROJECT_DIR = fs::canonical("/proc/self/exe").parent_path().parent_path();
fs::path       shader_file() { return PROJECT_DIR / "shaders" / "artcode.frag"; };

// shader lines of init, idx that contains version and layout keywords 0 -> 3
static constexpr int SHADER_DECLARATIONS_IDX = 3;

// targets for index identification
const string FUNC_TARGET   = "void main()";
const string VAR_TARGET    = "vec4 color = vec4(1.0f);";
const string RETURN_TARGET = "out_color = color;";

static inline UMap init_lookup;
// num of times derived class is initialize
static inline int init_count = 0;

struct ShapeRegistry {
    static void register_shape(detail::IPen* shape) {
        if (active_classes.size() == 0)
            active_classes.reserve(20);

        active_classes.push_back(shape);
    }

    static std::vector<detail::IPen*> get_classes() { return active_classes; }

    static void delete_registry(detail::IPen* shape) {
        active_classes.erase(
            std::remove(active_classes.begin(), active_classes.end(), shape),
            active_classes.end());
    }

  private:
    static inline std::vector<detail::IPen*> active_classes;
};

using NJson = nlohmann::json;

struct InstanceTracking {
    static constexpr string key = "instances";

    static inline NJson    js;
    static inline fs::path sln_file;
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

void write_shader(const ArrayString& lines) {
    // write to file
    std::ofstream write(shader_file());
    for (const auto& line : lines) {
        write << line << "\n";
    }
};

string to_glsl_var(const string& name, const Vec4& color) {
    string glsl_code_var  = "float " + name + " = " + name + "();";
    glsl_code_var        += "if (" + name + " < " + "0.0f)" + "{";
    glsl_code_var += "color = vec4(" + ToString(color.r) + "," + ToString(color.g) + "," +
                     ToString(color.b) + "," + ToString(color.a) + ");}";

    return glsl_code_var;
};

int find_target(const ArrayString& lines, const string& target) {
    const auto itr = std::find(lines.begin(), lines.end(), target);

    if (itr == lines.end()) {
        assert("Target variable vec4 color not found!");
        return 0;
    }

    // actual line num in shader
    return std::distance(lines.begin(), itr);
};

UMap created_instances() {
    // search project dir for solution extension
    const auto itr = std::find_if(
        fs::directory_iterator(PROJECT_DIR), fs::directory_iterator{},
        [&](const auto& file) -> bool { return file.path().extension() == ".rcd"; });

    if (itr == fs::directory_iterator()) {
        assert("Cannot find solution file, either not in this project directory or "
               "deleted!");
    } else {
        InstanceTracking::sln_file = PROJECT_DIR / itr->path().filename();
        std::ifstream read(InstanceTracking::sln_file);
        InstanceTracking::js = NJson::parse(read);
    }

    return InstanceTracking::js[InstanceTracking::key].get<UMap>();
};

void track_instances() {
    // replace the entire unorderer map entirely regardless of func name or num of instance
    InstanceTracking::js[InstanceTracking::key] = init_lookup;
    std::ofstream write(InstanceTracking::sln_file);
    write << InstanceTracking::js.dump(4);
};

// TODO:update to use the vert shader
//  instances
using DrawCircle = Art::Circle;
using DrawQuad   = Art::Quad;

// Circle
DrawCircle::Circle() {
    // initialize at object creation
    init_count++;
    this->name     = "circle_" + ToString(init_count);
    this->radius   = 0.5f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    // allocate to the registry
    ShapeRegistry::register_shape(this);
};

DrawCircle::~Circle() { ShapeRegistry::delete_registry(this); };

string DrawCircle::to_glsl_func() const {
    string glsl_code_func = "float " + this->name + "()" + "{";
    glsl_code_func += "return length(artboard_pos-vec2(" + ToString(this->position.x) +
                      "," + ToString(this->position.y) + ")" +
                      ")-" + // vulkan is y inverse
                      ToString(this->radius) + ";";
    glsl_code_func += "}";

    return glsl_code_func;
};

// Quad
DrawQuad::Quad() {
    // initialize at object creation
    init_count++;
    this->name     = "quad_" + ToString(init_count);
    this->l        = 100.0f;
    this->w        = 100.0f;
    this->position = Vec2{200, 200};
    this->color    = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    this->stroke   = 1.0f;
    this->scale    = 1.0f;

    // allocate to the registry
    ShapeRegistry::register_shape(this);
};

DrawQuad::~Quad() { ShapeRegistry::delete_registry(this); };

string DrawQuad::to_glsl_func() const {
    string glsl_code_func = "float " + this->name + "()" + "{";
    glsl_code_func += "vec2 d=abs(artboard_pos-vec2(" + ToString(this->position.x) + "," +
                      ToString(this->position.y) + "))" + "-" + "vec2(" +
                      ToString(this->w) + "," + ToString(this->l) + ");";
    glsl_code_func += "return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);}";

    return glsl_code_func;
};

void Art::Draw() {
    auto lines = read_lines();

    const auto active_classes = ShapeRegistry::get_classes();
    // only deletes the lines where functions and vars are
    {
        const auto instances = created_instances();
        if (active_classes.size() < instances.size()) {
            // index of func where it is inserted, +1 as arrays are 0 base
            const int func_start = SHADER_DECLARATIONS_IDX + 1;
            int       func_end   = func_start;

            // increment func_end until void main is found
            while (lines[func_end].find(FUNC_TARGET) == string::npos) {
                func_end++;
            }

            // delete all functions
            lines.erase(lines.begin() + func_start, lines.begin() + func_end);

            const int var_start = find_target(lines, VAR_TARGET) + 1;
            int       var_end   = var_start;

            while (lines[var_end].find(RETURN_TARGET) == string::npos) {
                var_end++;
            }

            // delete all variables
            lines.erase(lines.begin() + var_start, lines.begin() + var_end);
        }
    }

    // check num of instances
    for (int i = 0; i < active_classes.size(); i++) {
        const auto instance = active_classes[i];
        {
            const auto glsl_code_func = instance->to_glsl_func();
            // always +1 as arrays are 0 base, this guarantees to insert below the shader
            const int func_idx = SHADER_DECLARATIONS_IDX + (i + 1);
            // assign func name to look, saves the index as key, return type and func name
            init_lookup[func_idx] = glsl_code_func.substr(0, glsl_code_func.find("()"));

            // only write to shader if there is actual changes
            if (lines[func_idx] != glsl_code_func) {
                // insert functions before void main
                if (lines[func_idx].find(FUNC_TARGET) != string::npos) {
                    // insert before main at fresh creation
                    lines.insert(lines.begin() + func_idx, glsl_code_func);
                } else {
                    // replace entire code
                    lines.at(func_idx) = glsl_code_func;
                }
            }
        }

        // FIXME:this doesnt work at fresh shader file (no func, no var inserted)
        // first instance declared to be the first shape in 1st layer
        {
            const auto glsl_code_var = to_glsl_var(instance->name, instance->color);
            const auto target_idx    = find_target(lines, VAR_TARGET) + 1;
            const int  val_idx       = target_idx + (active_classes.size() - 1 - i);

            if (lines[val_idx] == glsl_code_var)
                continue;

            if (lines[val_idx].find(RETURN_TARGET) != string::npos) {
                lines.insert(lines.begin() + val_idx, glsl_code_var);
            } else {
                lines.at(val_idx) = glsl_code_var;
            }
        }
    }
    // write in shader all at once
    write_shader(lines);

    // write the new instances
    track_instances();
};
