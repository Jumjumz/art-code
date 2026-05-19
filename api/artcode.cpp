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

// shader lines of init, idx that contains version and layout keywords 0 -> 3
static constexpr int SHADER_DECLARATIONS_IDX = 3;

static inline UMap init_lookup;
// num of times derived class is initialize
static inline int init_count = 0;

fs::path shader_file() { return PROJECT_DIR / "shaders" / "artcode.frag"; };

using NJson = nlohmann::json;

struct ShapeRegistry {
    static void register_shape(Art::detail::IPen* shape) {
        if (active_classes.size() == 0)
            active_classes.reserve(20);

        active_classes.push_back(shape);
    }

    static std::vector<Art::detail::IPen*> get_classes() { return active_classes; }

    static void delete_registry(Art::detail::IPen* shape) {
        active_classes.erase(
            std::remove(active_classes.begin(), active_classes.end(), shape),
            active_classes.end());
    }

  private:
    static inline std::vector<Art::detail::IPen*> active_classes;
};

struct InstanceTracking {
    static constexpr string key = "instances";

    static inline NJson    js;
    static inline fs::path sln_file;
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

void insert_functions(ArrayString* lines, int line_idx, const string& glsl_code_func) {
    if ((*lines)[line_idx].find("void main") != string::npos) {
        // insert before main at fresh creation
        lines->insert(lines->begin() + line_idx, glsl_code_func);
    } else {
        // replace entire code
        lines->at(line_idx) = glsl_code_func;
    }
};

string to_glsl_var(const string& name, const Vec4& color) {
    string glsl_code_var  = "float " + name + " = " + name + "();";
    glsl_code_var        += "if (" + name + " < " + "0.0f)" + "{";
    glsl_code_var += "color = vec4(" + ToString(color.r) + "," + ToString(color.g) + "," +
                     ToString(color.b) + "," + ToString(color.a) + ");}";

    return glsl_code_var;
};

void insert_variables(ArrayString* lines) {
    // find this line
};

void Art::Draw() {
    auto lines = read_lines();

    const auto active_classes = ShapeRegistry::get_classes();

    {
        // get previous state of instances created
        const auto instances = created_instances();
        // delete all functions
        if (active_classes.size() < instances.size()) {
            for (int i = 0; i < instances.size(); i++) {
                const int line_idx = SHADER_DECLARATIONS_IDX + i;
                if (instances.contains(line_idx))
                    lines.erase(lines.begin() + line_idx);
            }
        }
    }

    // check num of instances
    for (int i = 0; i < active_classes.size(); i++) {
        const auto glsl_code_func = active_classes[i]->to_glsl_func();
        // always +1 for precise insert in line 5 for 1st instance and so on
        const int line_idx = SHADER_DECLARATIONS_IDX + (i + 1);
        // assign func name to look
        init_lookup[line_idx] = glsl_code_func.substr(0, glsl_code_func.find("()"));
        // only write to shader if there is actual changes
        if (lines[line_idx] == glsl_code_func)
            continue;

        // insert functions before void main
        // insert_functions(&lines, line_idx, glsl_code_func);
        if (lines[line_idx].find("void main") != string::npos) {
            // insert before main at fresh creation
            lines.insert(lines.begin() + line_idx, glsl_code_func);
        } else {
            // replace entire code
            lines.at(line_idx) = glsl_code_func;
        }
    }

    {
        // find this code in array
        const string TARGET = "  vec4 color = vec4(1.0f);";

        const auto itr = std::find(lines.begin(), lines.end(), TARGET);

        int idx;
        if (itr != lines.end()) {
            // actual line num in shader
            idx = std::distance(lines.begin(), itr) + 1;
        }

        for (int i = 0; i < active_classes.size(); i++) {
            const int line_idx = idx + i;
            // get current instance of
            const auto instance      = active_classes[i];
            const auto glsl_code_var = to_glsl_var(instance->name, instance->color);

            if (lines[line_idx] == glsl_code_var)
                continue;
            // FIXME:this is wrong..
            if (lines[line_idx].find(glsl_code_var) == string::npos) {
                lines.insert(lines.begin() + line_idx, glsl_code_var);
            } else {
                lines.at(line_idx) = glsl_code_var;
            }
        }
    }
    // write in shader all at once
    write_shader(lines);

    // write the new instances
    track_instances();
};

using DrawCircle = Art::Circle;

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
    glsl_code_func += "return length(artboard_pos - vec2(" + ToString(this->position.x) +
                      "," + ToString(this->position.y) + ")" +
                      ") - " + // vulkan is y inverse
                      ToString(this->radius) + ";";
    glsl_code_func += "}";

    return glsl_code_func;
};
