#version 450

layout(binding = 0) uniform ArtboardBuffer {
  mat4 proj;
  mat4 view;
  mat4 model;
  vec2 reso;
} ubo;

layout(location = 0) in vec2 artboard_pos;

layout(location = 0) out vec4 frag_color;

float star(vec2 p, float r) {
  const float k1x = 0.809016994f;
  const float k2x = 0.309016994f;
  const float k1y = 0.587785252f;
  const float k2y = 0.951056516f;
  const float k1z = 0.726542528f;
  const vec2 v1 = vec2( k1x, -k1y);
  const vec2 v2 = vec2(-k1x, -k1y);
  const vec2 v3 = vec2( k2x, -k2y);

  p.x = abs(p.x);
  p -= 2.0f * max(dot(v1, p), 0.0f) * v1;
  p -= 2.0f * max(dot(v2, p), 0.0f) * v2;
  p.x = abs(p.x);
  p.y -= r;

  return length(p-v3 * clamp(dot(p, v3), 0.0f, k1z * r)) * sign(p.y * v3.x - p.x * v3.y);
}

float circle(vec2 p, float r) {
  return length(p) - r;
}

void main() {
  const vec2 center = vec2(ubo.reso.x / 2, ubo.reso.y / 2);
  const float circle = circle(artboard_pos - center, 100.0f);
  const float star = star(artboard_pos - center, 200.0f);

  // cicle defines the position, 100.0f is the size of the circle
  if (circle < 0.0f) {
    frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    return;
  }

  if (star < 0.0f) {
    frag_color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    return;
  }

  frag_color = vec4(1.0f);
}
