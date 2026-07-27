#include "build.hpp"

#include "json.hpp"
#include <fstream>
#include <iostream>
#include <ostream>

Build::Build() {};

bool Build::set_project_directory(const fs::path& dir, const glm::vec3& artboard) {
    this->project_directory = dir;
    this->artboard_size     = artboard;

    return create_project_content();
};

bool Build::create_project_content() const {
    const std::vector<fs::path> content_directories = {
        this->project_directory / "build", this->project_directory / "shaders",
        this->project_directory / "components"};
    const auto& file_name = this->project_directory.filename().string() + this->sln_ext;
    const auto& solution_path = this->project_directory / file_name;

    // create sub directories
    if (!fs::exists(solution_path)) {
        // create and write solution file and main.cpp
        {
            std::vector<fs::path> project_content = {
                solution_path, this->project_directory / "main.cpp"};

            // create files
            for (const auto& content : project_content) {
                std::ofstream file(content);
            }
            // write to respective files
            write_solution_file(project_content[0]);
            write_main_cpp(project_content[1]);
        }

        // create directories
        for (const auto& directory : content_directories) {
            fs::create_directory(directory);
            std::cerr << directory << ": directory created" << std::endl;
        }

        {
            // create files inside respective directories
            const std::array<fs::path, 3> shaders = {
                content_directories[1] / "artcode.vert",
                content_directories[1] / "artcode.frag",
            };

            for (const auto& shader : shaders) {
                std::ofstream shader_file(shader);
            }
            const auto components_dir = content_directories[2];

            std::array<fs::path, 2> comp_files = {components_dir / "comp.hpp",
                                                  components_dir / "comp.cpp"};
            for (const auto& comp : components_dir) {
                std::ofstream file(comp);
            }

            // write comp files
            write_comp_hpp(comp_files[0]);
            write_comp_cpp(comp_files[1]);

            // write shaders for triangle and line topology
            write_vert_shader(shaders[0]);

            write_frag_shader(shaders[1]);
        }

        // create config folder
        create_config_dir();

        return true;
    } else {
        std::cerr << solution_path << " project solution already exist" << std::endl;
        return false;
    }
};

void Build::create_config_dir() const {
    if (!fs::exists(this->config_dir.parent_path())) {
        fs::create_directory(this->config_dir.parent_path());
    }

    // prepare to read and parse the projects.json file
    nlohmann::json js;
    if (fs::exists(this->config_dir)) {
        // read projects.json and parse
        std::ifstream read(this->config_dir);
        js = nlohmann::json::parse(read);
    } else {
        js["project_directory"] = nlohmann::json::array();
    }

    nlohmann::json path = js["project_directory"];

    // check if new created path already exist.. skips if it is
    if (std::find(path.begin(), path.end(), this->project_directory.string()) ==
        path.end()) {
        // delete the first item if array size is 10
        if (path.size() == 10) {
            js["project_directory"].erase(js["project_directory"].begin());
        }

        // append new project directory
        js["project_directory"].push_back(this->project_directory);

        std::ofstream write(this->config_dir);
        write << js.dump(4);
    }
};

void Build::write_solution_file(const fs::path& solution_file) const {
    // init json
    nlohmann::json js = {{"project_path", solution_file.parent_path()},
                         {"solution_file", solution_file.filename()},
                         {
                             "artboard_size",
                             {{"width", this->artboard_size.x},
                              {"height", this->artboard_size.y},
                              {"ppi", this->artboard_size.z}},
                         },
                         {"sources", {"main.cpp", "components/comp.cpp"}},
                         {"includes", nlohmann::json::array()},
                         {"shaders", {"shaders/artcode.vert", "shaders/artcode.frag"}}};

    // write
    std::ofstream write(solution_file);
    write << js.dump(4); // indent 4 spaces
};

void Build::write_main_cpp(const fs::path& main_cpp) const {
    std::ofstream write(main_cpp);
    // write init code in strign literal
    write << R"(#include <artcode.hpp> 
#include <iostream>
#include "components/comp.hpp"

int main() {
    Component comp;

    std::cout << comp.x << std::endl;

    return 0;
};)";
};

void Build::write_comp_cpp(const fs::path& comp) const {
    std::ofstream write(comp);
    // write init code in strign literal
    write << R"(#include "comp.hpp"

Component::Component() {
    this->x = 10;
};)";
};

void Build::write_comp_hpp(const fs::path& comp) const {
    std::ofstream write(comp);
    // write init code in strign literal
    write << R"(#pragma once

class Component {
    public:
      Component();

      int x;
    private:
};)";
};

// writes both triangle and line topology shaders
void Build::write_vert_shader(const fs::path& shader) const {
    std::ofstream write(shader);

    write << R"(#version 450
layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;vec2 viewport;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;float stroke;float rotate;int fill;int skew;} constant;
struct SkewPos{vec2 pos;int index;};
struct SkewData{vec2 skew_mesh[8]; SkewPos skew_pos[8];};
layout(std430, set = 0, binding = 1) readonly buffer SkewBuffer {SkewData data;} ssbo;
layout(location = 0) in vec2 in_pos;
vec2 calc_uv(vec2 pt, vec2 c0, vec2 c1, vec2 c2, vec2 c3) {
vec2 min_bound = min(min(c0, c1), min(c2, c3));
vec2 max_bound = max(max(c0, c1), max(c2, c3));
vec2 size = max_bound - min_bound;
return (pt - min_bound) / size;
}
vec2 bilinear(vec2 uv, vec2 c0, vec2 c1, vec2 c2, vec2 c3) {
vec2 top = mix(c0, c1, uv.x);
vec2 bot = mix(c3, c2, uv.x);
return mix(top, bot, uv.y);
}
vec2 skew(vec2 pos) {
vec2 c0 = ssbo.data.skew_mesh[0];
vec2 c1 = ssbo.data.skew_mesh[2];
vec2 c2 = ssbo.data.skew_mesh[4];
vec2 c3 = ssbo.data.skew_mesh[6];
vec2 def_c0 = c0;
vec2 def_c1 = c1;
vec2 def_c2 = c2;
vec2 def_c3 = c3;
for (int i = 0; i < 8; i++) {
int idx = ssbo.data.skew_pos[i].index;
vec2 offset = ssbo.data.skew_pos[i].pos;
if (idx == 0) def_c0 += offset;
if (idx == 1) { def_c0 += vec2(offset.x, 0.0); def_c1 += vec2(offset.x, 0.0);}
if (idx == 2) def_c1 += offset;
if (idx == 3) { def_c1 += vec2(0.0, offset.y); def_c2 += vec2(0.0, offset.y);}
if (idx == 4) def_c2 += offset;
if (idx == 5) { def_c2 += vec2(offset.x, 0.0); def_c3 += vec2(offset.x, 0.0);}
if (idx == 6) def_c3 += offset;
if (idx == 7) { def_c3 += vec2(0.0, offset.y); def_c0 += vec2(0.0, offset.y);}
}
vec2 uv = calc_uv(pos, c0, c1, c2, c3);
return bilinear(uv, def_c0, def_c1, def_c2, def_c3);
}
vec2 rotate(vec2 pos) {
const float PI = 3.14159265359;
float radian = constant.rotate * (PI / 180.0f);
float s = sin(radian);
float c = cos(radian);
vec2 center = constant.center;
center.y = ubo.reso.y - center.y;
pos -= center;
vec2 rotated = vec2(pos.x * c - pos.y * s, pos.x * s + pos.y * c);
return rotated + center;
}
void main() {
vec2 art_pos = in_pos;
art_pos.y = ubo.reso.y - art_pos.y;
if (constant.skew == 1) {
art_pos = skew(art_pos);
}
if (constant.rotate != 0) {
art_pos = rotate(art_pos);
}
gl_Position = ubo.proj * ubo.view * ubo.model * vec4(art_pos, 0.0f, 1.0f);
})";
};

void Build::write_frag_shader(const fs::path& shader) const {
    std::ofstream write(shader);

    write << R"(#version 450
layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;vec2 viewport;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;float stroke;float rotate;int fill;int skew;} constant;
layout(location = 0) out vec4 frag_color;
void main() {
vec3 color = constant.color.rgb;
color = pow(color, vec3(2.2));
frag_color = vec4(color, constant.color.a);
})";
};
