#version 450

layout(binding = 0) uniform ArtboardBuffer {
  mat4 proj;
  mat4 view;
  mat4 model;
  vec2 reso;
  float ppi;
} ubo;

layout(location = 0) in vec2 artboard_pos;

layout(location = 0) out vec4 frag_color;

void main() {
  frag_color = vec4(1.0f);
}
