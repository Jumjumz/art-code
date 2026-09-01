#version 450

layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;float ppi;} ubo;
layout(push_constant) uniform PushConstants {
  vec4 color;
  vec2 pos;
  vec2 center;
  vec2 shape_data;
  float stroke;
  float rotate;
  int fill;
  int skew;
  int shape_type;
} constant;
// layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;
layout(location = 0) in vec2 vert_pos;

// From Inigo Quilez (curve line sdf)
float sd_bezier(vec2 pos, vec2 p0, vec2 p1, vec2 p2) {
  vec2 a = p1 - p0;
  vec2 b = p0 - 2.0f * p1 + p2;
  vec2 c = a * 2.0f;
  vec2 d = p0 - pos;

  float kk = 1.0f / dot(b, b);
  float kx = kk * dot(a, b);
  float ky = kk * (2.0f * dot(a, a) + dot(d, b)) / 3.0f;
  float kz = kk * dot(d, a);

  float res = 0.0f;
  float p = ky - kx * kx;
  float q = kx * (2.0f * kx * kx - 3.0f * ky) + kz;
  float p3 = p * p * p;
  float q2 = q * q;
  float h = q2 + 4.0f * p3;

  if (h >= 0.0f) {
    h = sqrt(h);
    vec2 x = (vec2(h, -h) - q) / 2.0f;
    vec2 uv2 = sign(x) * pow(abs(x), vec2(1.0f / 3.0f));
    float t = clamp(uv2.x + uv2.y - kx, 0.0f, 1.0f);
    vec2 q2 = d + (c + b * t) * t;

    res = dot(q2, q2);
  } else {
    float z = sqrt(-p);
    float v = acos(q / (p * z * 2.0f)) / 3.0f;
    float m = cos(v);
    float n = sin(v) * 1.732050808f;
    vec3 t2 = clamp(vec3(m + m, -n - m, n - m) * z - kx, 0.0f, 1.0f);
    vec2 qx = d + (c + b * t2.x) * t2.x;
    float dx = dot(qx, qx);
    vec2 qy = d + (c + b * t2.y) * t2.y;
    float dy = dot(qy, qy);

    res = (dx < dy) ? dx : dy;
  }

  return sqrt(res);
}

// shapes
float sd_quad(vec2 p, vec2 b) {
  vec2 n_b = b * 0.5;
  vec2 d = abs(p) - n_b;

  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

float sd_circle(vec2 p, float r) {
  return length(p) - r;
}

//TODO:apply bezier sdf for line topology
void main() {
  vec3 color = constant.color.rgb;
  // convert color to linear space using gamma correction 2.2
  color = pow(color, vec3(2.2f));

  int shape = constant.shape_type;
  vec2 shape_data = constant.shape_data;
  vec2 pos = vert_pos;
  pos.y = ubo.reso.y - pos.y;

  vec2 p = pos;
  float d = 1.0f;

  float alpha = constant.color.a;
  // uses loop-blinn for quadratic curves
  /*if (uv.x != 10.0f || uv.y != 10.0f) {
    if (constant.fill == 1) {
      // quadratic bezier
      float f = uv.x * uv.x - uv.y;
      float fw = fwidth(f);

      alpha -= smoothstep(-fw, fw, f);
    } else {
      //NOTE:this now works as it only does triangle list topology
      vec2 p0 = constant.p0;
      vec2 p1 = constant.p1;
      vec2 p2 = constant.p2;

      float dist = sd_bezier(pos, p0, p1, p2);
      float stroke = constant.stroke;
      float fw = fwidth(dist);

      alpha -= smoothstep(stroke - fw, stroke + fw, dist);
    }
  }*/

  // renders correct shape
  if (constant.fill == 1) {
    if (shape == 0) {
      d = sd_quad(p, shape_data);
    } else if (shape == 1) {
      d = sd_circle(p, shape_data.x);
    }
  }

  // discard outside
  if (d > 0.0f) discard;
 
  // only renders the curve inside the triangle
  // if(alpha < 0.001f) discard;

  frag_color = vec4(color, alpha);
}
