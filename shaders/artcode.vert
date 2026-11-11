#version 450

layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;float ppi;float zoom;} ubo;
layout(push_constant) uniform PushConstants {
  vec4 bg_color;
  vec4 color;
  vec2 pos;
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
struct Distort{vec2 pos;int index;};
layout(std430, set = 0, binding = 1) readonly buffer DistortBuffer {Distort data[];} distort_ssbo;
layout(location = 0) out vec2 shape_pos;
layout(location = 1) out vec2 distort_pos;
layout(location = 2) out vec2 mesh_center;
layout(location = 3) flat out int is_distorted;
layout(location = 4) flat out mat3 h_inv;
layout(location = 7) flat out float padding;
layout(location = 8) out vec2 mesh_tl;
layout(location = 9) out vec2 final_mesh_size;

// forward declarations
vec2 grid_uv         ();
vec2[6] shape_mesh   ();
vec2[6] distort_mesh ();
mat3 square_to_quad  (vec2 q0, vec2 q1, vec2 q2, vec2 q3);
// helper functions
vec2 get_center          (vec2 positions[6]);
vec2 rotate              (vec2 pos, vec2 center);
vec2 bilinear            (vec2 uv, vec2 tl, vec2 tr, vec2 bl, vec2 br);

void main() {
  vec2 uv = grid_uv();
  // NOTE:separated the mesh positions and the skewed positions
  const vec2 mesh_positions[6] = shape_mesh();
  const vec2 dist_positions[6] = distort_mesh();

  vec2 dst_pos = bilinear(uv, dist_positions[0], dist_positions[1], dist_positions[2], dist_positions[5]);

  int distorted = 0;
  vec2 center  = vec2(0.0f);
  // index = -1 is default value for non skew
  if (distort_ssbo.data[0].index != -1)
    distorted = 1;

  if (constant.rotate != 0.0f) {
    // get center
    center  = get_center(dist_positions);
    dst_pos = rotate(dst_pos, center);
  }
  // pass to frag shader
  is_distorted       = distorted;
  mesh_center        = center;
  distort_pos        = dst_pos;
  shape_pos          = bilinear(uv, mesh_positions[0], mesh_positions[1], mesh_positions[2], mesh_positions[5]);
  mat3 H             = square_to_quad(dist_positions[0], dist_positions[1], dist_positions[5], dist_positions[2]);
  h_inv              = inverse(H);
  mesh_tl            = mesh_positions[0];
  final_mesh_size    = mesh_positions[5] - mesh_positions[0];
  // pixels will adjust depending on the skew values
  // no skew means rect/square mesh
  gl_Position = ubo.proj * ubo.view * ubo.model * vec4(distort_pos, 0.0f, 1.0f);
}

vec2 grid_uv() {
  const int GRID = 12;

  const vec2 offsets[6] = vec2[](
      vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
      vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f)
  );
  int cell   = gl_VertexIndex / 6;
  int corner = gl_VertexIndex % 6;
  // cell column, row
  vec2 c = vec2(cell % GRID, cell / GRID);
  return (c + offsets[corner]) / float(GRID);
}

// implementations
vec2[6] shape_mesh() {
// NOTE:add stroke in padding for pen instances
  // this fixes render issues for stroke and mesh size
  // making the mesh size dynamic depending on stroke value
  const vec2 mesh_size = constant.mesh_size;
  vec2 position        = constant.pos;

  padding = (constant.fill == 0 && constant.shape_type == 3) ? constant.stroke + 20.0f : 20.0f;
  // set position to ubo coord
  position.y += ubo.reso.y;
  vec2 positions[6] = vec2[](
    vec2( position.x - padding, position.y - padding),
    vec2( ( position.x + mesh_size.x ) + padding, position.y - padding  ),
    vec2( position.x - padding,  ( position.y + mesh_size.y ) + padding ),
    vec2( ( position.x + mesh_size.x ) + padding, position.y - padding  ),
    vec2( position.x - padding,  ( position.y + mesh_size.y ) + padding ),
    ( position + mesh_size ) + padding
  );

  return positions;
}

vec2[6] distort_mesh() {
  vec2[6] positions = shape_mesh();
  for (int i = 0; i < distort_ssbo.data.length(); i++) {
    const vec2 pos = distort_ssbo.data[i].pos;
    const int idx  = distort_ssbo.data[i].index;
    // exit at index default
    if (idx == -1)
      break;

    switch(idx) {
      case 0:
        positions[0] += pos;
        positions[5].y -= pos.x / 2.0f;
        break;
      case 1:
        positions[0].x += pos.x;
        positions[1].x += pos.x;
        positions[3].x += pos.x;

        break;
      case 2:
        positions[1] += pos;
        positions[3] += pos;
        positions[2].y += pos.x / 2.0f;
        positions[4].y += pos.x / 2.0f;
        break;
      case 3:
        positions[1].y += pos.y;
        positions[3].y += pos.y;
        positions[5].y += pos.y;
        break;
      case 4:
        positions[5] += pos;
        break;
      case 5:
        positions[2].x += pos.x;
        positions[4].x += pos.x;
        positions[5].x += pos.x;
        break;
      case 6:
        positions[2] += pos;
        positions[4] += pos;
        positions[1].y += pos.x / 2.0f;
        positions[3].y += pos.x / 2.0f;
        break;
      case 7:
        positions[0].y += pos.y;
        positions[2].y += pos.y;
        positions[4].y += pos.y;
        break;
    }
  }

  return positions;
}

mat3 square_to_quad(vec2 q0, vec2 q1, vec2 q2, vec2 q3) {
  vec2 d1 = q1 - q2;
  vec2 d2 = q3 - q2;
  vec2 d3 = q0 - q1 + q2 - q3;

  float g = 0.0f, h = 0.0f;
  if (abs(d3.x) > 1e-6 || abs(d3.y) > 1e-6) {
    const float den = d1.x * d2.y - d2.x * d1.y;
    g = (d3.x * d2.y - d2.x * d3.y) / den;
    h = (d1.x * d3.y - d3.x * d1.y) / den;
  }

  vec2 a = q1 - q0 + g * q1; // u axis
  vec2 b = q3 - q0 + h * q3; // v axis

  return mat3( a.x,  a.y,  g,
               b.x,  b.y,  h,
              q0.x, q0.y,  1.0f
      );
}

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

vec2 bilinear(vec2 uv, vec2 tl, vec2 tr, vec2 bl, vec2 br) {
  return mix(mix(tl, tr, uv.x), mix(bl, br, uv.x), uv.y);
}
