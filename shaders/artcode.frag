#version 450

layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;float ppi;} ubo;
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
struct PenData {vec2 position;vec2 uv;int id;};
layout(std430, set = 0, binding = 2) readonly buffer PenBuffer {PenData data[];} pen_ssbo;
layout(location = 0) in vec2 vert_pos;
layout(location = 0) out vec4 frag_color;

// forward declarations
float sdf_quad     (vec2 p, vec2 b);
float sdf_circle   (vec2 p, float r);
float sdf_triangle (vec2 p, vec2 p0, vec2 p1, vec2 p2);
float sdf_segment  (vec2 p, vec2 a, vec2 b);
float sdf_bezier   (vec2 p, vec2 p0, vec2 p1, vec2 p2);

int ray_cross      (vec2 p, vec2 a, vec2 b);
int sdf_winding    (vec2 p, vec2 p0, vec2 p1, vec2 p2);

void main() {
  vec3 color  = constant.color.rgb;
  float alpha = constant.color.a;
  // convert color to linear space using gamma correction 2.2
  color = pow(color, vec3(2.2f));

  int shape = constant.shape_type;

  // init variables
  vec2 shape_data = constant.shape_data;
  vec2 pos        = constant.pos;
  vec2 center     = vec2(0.0f, 0.0f);
  vec2 p          = vec2(0.0f, 0.0f);
  // set pos to ubo coord
  pos.y = ubo.reso.y + pos.y;

  float stroke = constant.stroke;
  float d      = 1e10; // large number for pen to work
  int winding  = 0;    // for pen fill instances
  if (shape == 0) {
    const vec2 b = shape_data * 0.5f;
    center       = pos + b;
    p            = vert_pos - center;

    d = sdf_quad(p, b);
  } else if (shape == 1) {
    center = pos + shape_data;
    p      = vert_pos - center;

    d = sdf_circle(p, shape_data.x);
  } else if (shape == 2) {
    vec2 p0 = constant.p0;
    vec2 p1 = constant.p1;
    vec2 p2 = constant.p2;

    p0.y += ubo.reso.y;
    p1.y += ubo.reso.y;
    p2.y += ubo.reso.y;

    // free form triangle
    d = sdf_triangle( vert_pos, p0, p1, p2 );
  } else if (shape == 3) {
    int i = 0;
    while (i < pen_ssbo.data.length() - 1) {
      int id = pen_ssbo.data[i].id;
      if (id == 0) {
        vec2 p0 = pen_ssbo.data[i].position;
        vec2 p1 = pen_ssbo.data[i + 1].position; // handle
        vec2 p2 = pen_ssbo.data[i + 2].position;

        // normalize to ubo cooridnates
        p0.y += ubo.reso.y;
        p1.y += ubo.reso.y;
        p2.y += ubo.reso.y;

        if (constant.fill == 0) {
          d = min(d, sdf_bezier( vert_pos, p0, p1, p2 ));
        } else {
          winding += sdf_winding( vert_pos, p0, p1, p2 );
        }
        // i+2, adding it here makes i jumps to 3rd item
        i += 2;
      } else {
        vec2 a = pen_ssbo.data[i].position;
        vec2 b = pen_ssbo.data[i + 1].position;

        // normalize to ubo cooridnates
        a.y += ubo.reso.y;
        b.y += ubo.reso.y;

        if (constant.fill == 0) {
          // NOTE:enclosing a render doesnt display the full stroke width
          // it only displays half of it making the render not consistent
          // across segments, dont know how to fix this yet, might play
          // around the mesh and the render area
          d = min(d, sdf_segment( vert_pos, a, b ));
        } else {
          winding += ray_cross( vert_pos, a, b );
        }
        i++;
      }
    }
  }
 
  // discard outside shape, aka the mesh
  // pen has different rendering
  if (shape != 3 && d > 0.0f) discard;

  // FIXME:this renders a jagged filled pen
  // only for rendering fill pen shapes
  if (constant.fill == 1 && shape == 3 && winding == 0) discard;

  // NOTE:trying to find a way to render curves
  // that fill look like a horizontal/vertical lines
  const float AA_SPREAD = fwidth(d);

  // NOTE:excempt quad and curve lines
  // for curve lines it causes a bug where it has transparent
  // curve in the middle of the curve
  if (shape != 0 && shape != 3) {
    alpha -= smoothstep(-AA_SPREAD, AA_SPREAD, d);
  }
 
  // render annular shapes
  if (constant.fill == 0) {
    // pen doesnt follow the same check for discarding
    if (shape == 3) {
      if (d > stroke) discard;
    } else {
      // discard inner to create an annular shape
      if (abs(d) > stroke) discard;
    }

    // excempt quads for anti-aliasing
    if (shape != 0) {
      // anti-aliasing for inner edge
      alpha -= smoothstep(stroke - AA_SPREAD, stroke + AA_SPREAD, abs(d));
    }
  }

  // NOTE:debugging purpose
  // if (d > 0.0f) color = vec3(alpha);

  // only renders the curves
  // if (alpha < 0.00001f) discard;

  frag_color = vec4(color, alpha);
  // NOTE:testing mixing colors
  // frag_color = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(color, alpha), alpha);
}

// shapes From Inigo Quilez
float sdf_quad(vec2 p, vec2 b) {
  const vec2 d = abs(p) - b;

  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f);
}

float sdf_circle(vec2 p, float r) {
  return length(p) - r;
}

// free form triangle
float sdf_triangle(vec2 p, vec2 p0, vec2 p1, vec2 p2) {
  vec2 e0 = p1 - p0;
  vec2 e1 = p2 - p1;
  vec2 e2 = p0 - p2;
  vec2 v0 =  p - p0;
  vec2 v1 =  p - p1;
  vec2 v2 =  p - p2;

  vec2 pq0 = v0 - e0 * clamp( dot(v0, e0) / dot(e0, e0), 0.0f, 1.0f );
  vec2 pq1 = v1 - e1 * clamp( dot(v1, e1) / dot(e1, e1), 0.0f, 1.0f );
  vec2 pq2 = v2 - e2 * clamp( dot(v2, e2) / dot(e2, e2), 0.0f, 1.0f );

  float s = sign( e0.x * e2.y - e0.y * e2.x );
  vec2  d = min( min( vec2( dot( pq0, pq0 ), s * ( v0.x * e0.y - v0.y * e0.x ) ),
                      vec2( dot( pq1, pq1 ), s * ( v1.x * e1.y - v1.y * e1.x ) ) ),
                      vec2( dot( pq2, pq2 ), s * ( v2.x * e2.y - v2.y * e2.x ) ) );

  return -sqrt(d.x) * sign(d.y);
}

float sdf_segment(vec2 p, vec2 a, vec2 b) {
  vec2 pa = p - a;
  vec2 ba = b - a;
  float h = clamp( dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);

  return length(pa - ba * h);
}

// curve line sdf
float sdf_bezier(vec2 pos, vec2 p0, vec2 p1, vec2 p2) {
  vec2 a = p1 - p0;
  vec2 b = p0 - 2.0f * p1 + p2;
  vec2 c = a * 2.0f;
  vec2 d = p0 - pos;

  float kk  = 1.0f / dot(b, b);
  float kx  = kk * dot(a, b);
  float ky  = kk * (2.0f * dot(a, a) + dot(d, b)) / 3.0f;
  float kz  = kk * dot(d, a);
  float res = 0.0f;
  float p   = ky - kx * kx;
  float q   = kx * (2.0f * kx * kx - 3.0f * ky) + kz;
  float p3  = p * p * p;
  float q2  = q * q;
  float h   = q2 + 4.0f * p3;

  if (h >= 0.0f) {
    h        = sqrt(h);
    vec2 x   = (vec2(h, -h) - q) / 2.0f;
    vec2 uv2 = sign(x) * pow(abs(x), vec2(1.0f / 3.0f));
    float t  = clamp(uv2.x + uv2.y - kx, 0.0f, 1.0f);
    vec2 q2  = d + (c + b * t) * t;

    res = dot(q2, q2);
  } else {
    float z  = sqrt(-p);
    float v  = acos(q / (p * z * 2.0f)) / 3.0f;
    float m  = cos(v);
    float n  = sin(v) * 1.732050808f;
    vec3 t2  = clamp(vec3(m + m, -n - m, n - m) * z - kx, 0.0f, 1.0f);
    vec2 qx  = d + (c + b * t2.x) * t2.x;
    float dx = dot(qx, qx);
    vec2 qy  = d + (c + b * t2.y) * t2.y;
    float dy = dot(qy, qy);

    res = (dx < dy) ? dx : dy;
  }

  // returns negative value as y coord is in negative space
  return sqrt(res);
}

int sdf_winding(vec2 p, vec2 p0, vec2 p1, vec2 p2) {
  // quadratic for y intersection
  float a = p0.y - 2.0f * p1.y + p2.y;
  float b = 2.0f * (p1.y - p0.y);
  float c = p0.y - p.y;

  float discriminant = b * b - 4.0 * a * c;
  if (discriminant < 0.0f) return 0; // no intersection

  int winding = 0;
  float sqrt_d = sqrt(discriminant);

  // possible t values
  float t1 = (-b + sqrt_d) / (2.0f * a);
  float t2 = (-b - sqrt_d) / (2.0f * a);

  if (t1 >= 0.0f && t1 <= 1.0f) {
    float x = (1.0f - t1) * (1.0f - t1) * p0.x + 2.0f * (1.0f - t1) * t1 * p1.x + t1 * t1 * p2.x;
    if (x > p.x) {
      float dy = 2.0f * (1.0f - t1) * (p1.y - p0.y) + 2.0f * t1 * (p2.y - p1.y);
      winding += (dy > 0.0f) ? 1 : -1;
    }
  }

  if (t2 >= 0.0f && t2 <= 1.0f) {
    float x = (1.0f - t2) * (1.0f - t2) * p0.x + 2.0f * (1.0f - t2) * t2 * p1.x + t2 * t2 * p2.x;
    if (x > p.x) {
      float dy = 2.0f * (1.0f - t2) * (p1.y - p0.y) + 2.0f * t2 * (p2.y - p1.y);
      winding += (dy > 0.0f) ? 1 : -1;
    }
  }

  return winding;
}

int ray_cross(vec2 p, vec2 a, vec2 b) {
  if (a.y > p.y != b.y > p.y) {
    float x_intersect = a.x + (p.y - a.y) / (b.y - a.y) * (b.x - a.x);
    if (p.x < x_intersect)
      return (a.y < b.y) ? 1 : -1;
  }
  return 0;
}
