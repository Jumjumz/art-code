#version 450

layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;float ppi;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;vec2 p0;vec2 p1;vec2 p2;float stroke;float rotate;int fill;int skew;} constant;
struct SkewPos{vec2 pos;int index;};
struct SkewData{vec2 skew_mesh[8]; SkewPos skew_pos[8];};
layout(std430, set = 0, binding = 1) readonly buffer SkewBuffer {SkewData data;} ssbo;
layout(location = 0) in vec4 in_pos;
layout(location = 0) out vec2 uv;
layout(location = 1) out vec2 pos;

//TODO:remove and transfer these functions to frag shader,
// SDF will be the main core of rendering shapes from now on, that also means
// skew and rotate will be in frag shader
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
  vec2 art_pos = in_pos.xy;
  art_pos.y = ubo.reso.y - art_pos.y;
  if (constant.skew == 1) {
    art_pos = skew(art_pos);
  }
  if (constant.rotate != 0) {
    art_pos = rotate(art_pos);
  }

  // pass in_pos.wz to frag shader
  uv = in_pos.wz;
  // pass the current vertex
  pos = art_pos;

  gl_Position = ubo.proj * ubo.view * ubo.model * vec4(art_pos, 0.0f, 1.0f);
}
