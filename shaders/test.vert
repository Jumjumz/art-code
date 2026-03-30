#version 450

layout(binding = 0) uniform ArtboardBuffer {
  mat4 proj;
  mat4 view;
  mat4 model;
  vec2 reso;
} ubo;

void main() {
  // sets the entire canvas viewport to row: 0 -> width, column: 0 -> height
  const vec2 positions[4] = vec2[](
    vec2( 0.0f,  0.0f),
    vec2( ubo.reso.x,  0.0f),
    vec2( 0.0f,  ubo.reso.y),
    vec2( ubo.reso.x, ubo.reso.y)
  );

  gl_Position = ubo.proj * ubo.view * (ubo.model * vec4(positions[gl_VertexIndex], 0.0f, 1.0f));
}
