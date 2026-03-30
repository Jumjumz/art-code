#version 450

layout(binding = 0) uniform ArtboardBuffer {
  mat4 proj;
  mat4 view;
  mat4 model;
  vec2 reso;
} ubo;

layout(location = 0) in vec2 artboard_pos;

layout(location = 0) out vec4 frag_color;

void main() {
  const float circle = length(artboard_pos - vec2(ubo.reso.x / 2, ubo.reso.y / 2));

  // cicle defines the position, 100.0f is the size of the circle
  if (circle < 100.0f) {
    frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    return;
  }

  frag_color = vec4(1.0f);
}
