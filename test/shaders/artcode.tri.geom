#version 450
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;
layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;vec2 viewport;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;float stroke;float rotate;int fill;int skew;} constant;
struct SkewPos{vec2 pos;int index;};
struct SkewData{vec2 skew_mesh[8]; SkewPos skew_pos[8];};
layout(std430, set = 0, binding = 1) readonly buffer SkewBuffer {SkewData data;} ssbo;
layout(location = 0) in vec2 art_pos[];
layout(location = 1) out vec3 data;
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
vec2 ssbo_pos = ssbo.data.skew_pos[i].pos;
vec2 pts = vec2(ssbo_pos.x, -ssbo_pos.y);
if (idx == 0) def_c0 += pts;
if (idx == 1) { def_c0 += vec2(pts.x, 0.0); def_c1 += vec2(pts.x, 0.0);}
if (idx == 2) def_c1 += pts;
if (idx == 3) { def_c1 += vec2(0.0, pts.y); def_c2 += vec2(0.0, pts.y);}
if (idx == 4) def_c2 += pts;
if (idx == 5) { def_c2 += vec2(pts.x, 0.0); def_c3 += vec2(pts.x, 0.0);}
if (idx == 6) def_c3 += pts;
if (idx == 7) { def_c3 += vec2(0.0, pts.y); def_c0 += vec2(0.0, pts.y);}
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
vec2 pos_arr[] = art_pos;
for (int i = 0; i < gl_in.length(); i++) {
vec2 pos = pos_arr[i];
if (constant.skew == 1) {
pos = skew(pos);
}
if (constant.rotate != 0) {
pos = rotate(pos);
}
gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 0.0f, 1.0f);
data = vec3(1.0f);
EmitVertex();
}
EndPrimitive();
}