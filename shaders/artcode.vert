#version 450

layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;float ppi;float zoom;} ubo;
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
struct SkewPos{vec2 pos;int index;};
struct SkewData{vec2 skew_mesh[8]; SkewPos skew_pos[8];};
layout(std430, set = 0, binding = 1) readonly buffer SkewBuffer {SkewData data;} skew_ssbo;
layout(location = 0) out vec2 vert_pos;
layout(location = 1) out vec2 mesh_center;

//TODO:remove and transfer these functions to frag shader,
// SDF be the main core of rendering shapes from now on, that also means
// skew and rotate will be in frag shader
/*vec2 calc_uv(vec2 pt, vec2 c0, vec2 c1, vec2 c2, vec2 c3) {
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
}*/

// forward declarations
vec2 get_center (vec2 positions[6]);
vec2 rotate     (vec2 pos, vec2 center);

void main() {
  // NOTE:add stroke in padding for pen instances
  // this fixes render issues for stroke and mesh size
  // making the mesh size dynamic depending on stroke value
  const float padding  = (constant.fill == 0 && constant.shape_type == 3) ? constant.stroke + 20.0f : 20.0f;
  const vec2 mesh_size = constant.mesh_size;
  vec2 position        = constant.pos;
  // test skew ssbo
  const vec2 ssb_skew  = skew_ssbo.data.skew_pos[0].pos;
  // set position to ubo coord
  position.y += ubo.reso.y;

  // NOTE:might need to add padding in mesh
  // sets the quad to position and mesh size
  const vec2 positions[6] = vec2[](
    vec2( position.x - padding, position.y - padding),
    vec2( ( position.x + mesh_size.x ) + padding, position.y - padding  ),
    vec2( position.x - padding,  ( position.y + mesh_size.y ) + padding ),
    vec2( ( position.x + mesh_size.x ) + padding, position.y - padding  ),
    vec2( position.x - padding,  ( position.y + mesh_size.y ) + padding ),
    ( position + mesh_size ) + padding
  );

  vec2 art_pos = positions[gl_VertexIndex];
  if (constant.rotate != 0.0f) {
    vec2 center = get_center(positions); // get center
    mesh_center = center;
    art_pos = rotate(art_pos, center);
  }
  vert_pos = art_pos;

  gl_Position = ubo.proj * ubo.view * ubo.model * vec4(vert_pos, 0.0f, 1.0f);
}

// implementations
vec2 get_center(vec2 positions[6]) {
  vec2 center = vec2(0.0f, 0.0f);
  for (int i = 0; i < positions.length(); i++) {
    center += vec2(positions[i].x, positions[i].y);
  }
  return center /= float(positions.length());
}

vec2 rotate(vec2 pos, vec2 center) {
  const float PI       = 3.14159265359;
  const float radian   = constant.rotate * (PI / 180.0f);
  const float s        = sin(radian);
  const float c        = cos(radian);

  pos -= center;
  return vec2(pos.x * c - pos.y * s, pos.x * s + pos.y * c) + center;
}
