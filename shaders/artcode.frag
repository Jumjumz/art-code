#version 450

layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;float ppi;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;vec2 p0;vec2 p1;vec2 p2;float stroke;float rotate;int fill;int skew;} constant;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;
layout(location = 1) in vec2 pos;

// From Inigo Quilez
float bezier_sdf(vec2 pos, vec2 p0, vec2 p1, vec2 p2) {
  vec2 a = p1 - p0;
  vec2 b = p0 - 2.0 * p1 + p2;
  vec2 c = a * 2.0;
  vec2 d = p0 - pos;

  float kk = 1.0 / dot(b, b);
  float kx = kk * dot(a, b);
  float ky = kk * (2.0 * dot(a, a) + dot(d, b)) / 3.0;
  float kz = kk * dot(d, a);

  float res = 0.0;
  float p = ky - kx * kx;
  float q = kx * (2.0 * kx * kx - 3.0 *ky) + kz;
  float p3 = p * p * p;
  float q2 = q * q;
  float h = q2 + 4.0 * p3;

  if (h >= 0.0) {
    h = sqrt(h);
    vec2 x = (vec2(h, -h) - q) / 2.0;
    vec2 uv2 = sign(x) * pow(abs(x), vec2(1.0 / 3.0));
    float t = clamp(uv2.x + uv2.y - kx, 0.0, 1.0);
    vec2 q2 = d + (c + b * t) * t;
    res = dot(q2, q2);
  } else {
    float z = sqrt(-p);
    float v = acos(q / (p * z * 2.0)) / 3.0;
    float m = cos(v);
    float n = sin(v) * 1.732050808;
    vec3 t2 = clamp(vec3(m + m, -n - m, n - m) * z - kx, 0.0, 1.0);
    vec2 qx = d + (c + b * t2.x) * t2.x;
    float dx = dot(qx, qx);
    vec2 qy = d + (c + b * t2.y) * t2.y;
    float dy = dot(qy, qy);
    res = (dx < dy) ? dx : dy;
  }

  return sqrt(res);
}

//TODO:apply bezier sdf for line topology
void main() {
  vec3 color = constant.color.rgb;
  // convert color to linear space using gamma correction 2.2
  color = pow(color, vec3(2.2));

  float alpha = constant.color.a;
  // uses loop-blinn for quadratic curves
  if (uv.x != 10.0f || uv.y != 10.0f) {
    if (constant.fill == 1) {
      // quadratic bezier
      float f = uv.x * uv.x - uv.y;
      float fw = fwidth(f);

      alpha = constant.color.a - smoothstep(-fw, fw, f);
    } else {
      //doesnt work for line topology, might need to use triangles
      vec2 p0 = constant.p0;
      vec2 p1 = constant.p1;
      vec2 p2 = constant.p2;

      float dist = bezier_sdf(pos, p0, p1, p2);
      float stroke = constant.stroke;
      float fw = fwidth(dist);

      alpha = constant.color.a - smoothstep(stroke - fw, stroke + fw, dist);
    }
  }

  // only renders the curve inside the triangle
  if(alpha < 0.001f) discard;

  frag_color = vec4(color, alpha);
}
