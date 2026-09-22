#version 450

layout(binding = 0) uniform ArtboardBuffer {
  mat4 proj;
  mat4 view;
  mat4 model;
  vec2 reso;
  float ppi;
} ubo;

layout(push_constant) uniform PushConstants {
  vec4 bg_color;
  vec4 color;
  vec2 pos;
  vec2 center;
  vec2 shape_data;
  vec2 mesh_size;
  vec2 p0;
  vec2 p1;
  vec2 p2;
  float stroke;
  float rotate;
  int fill;
  int skew;
  int shape_type;
} constant;

layout(location = 0) in vec2 artboard_pos;

layout(location = 0) out vec4 frag_color;

void main() {
  vec3 color = pow(constant.bg_color.rgb, vec3(2.2f));
  float alpha = constant.bg_color.a;

  frag_color = vec4(color, alpha);
}
