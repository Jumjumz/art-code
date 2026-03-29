#version 450

layout(binding = 0) uniform ArtboardBuffer {
  mat4 proj;
  mat4 view;
  vec4 color;
  vec2 reso;
  vec2 mouse;
} ubo;

void main() { 
  vec2 positions[5] = vec2[](
    vec2( 0.0,  0.0),
    vec2( ubo.reso.x,  0.0),
    vec2( ubo.reso.x, ubo.reso.y),
    vec2( 0.0, ubo.reso.y),
    vec2( 0.0, 0.0)
  );

  gl_Position = ubo.proj * ubo.view * vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
