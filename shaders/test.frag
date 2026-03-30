#version 450

layout(binding = 0) uniform ArtboardBuffer {
  mat4 proj;
  mat4 view;
  mat4 model;
  vec2 reso;
} ubo;

layout(location = 0) out vec4 frag_color;

float sdf(vec3 p, vec3 b) {
  vec3 q = abs(p) - b;

  return length(max(q, 0.0f)) + min(max(q.x, max(q.x, q.z)), 0.0f);
}

float ray_march(vec3 ro, vec3 rd) {
  float dist = 0.0f; // distance from origin
  const int MAX_STEPS = 100;

  for (int i = 0; i < MAX_STEPS; i++) {
    vec3 p = ro + rd * dist;
    float ds = sdf(p, vec3(0.5f, 0.5f, 0.5f));
    dist += ds;

    if (dist > MAX_STEPS || ds < 0.001) break;
  }

  return dist;
}

void main() {
  // get artboard origin
  const vec4 ab_org = ubo.proj * ubo.model * vec4(0.0f, 0.0f, 0.0f, 1.0f);
 
  // convert to screen space pixels
  vec2 origin = (ab_org.xy * 0.5 + 0.5) * ubo.reso;

  // convert gl frag coord to artboard space
  vec2 ab_coord = gl_FragCoord.xy - origin;

  // clip, dont render outside the artboard
  if (ab_coord.x < 0.0 || ab_coord.x > ubo.reso.x || ab_coord.y < 0.0 || ab_coord.y > ubo.reso.y) {
    frag_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    return;
  }

  const vec2 uv = (ab_coord.xy * 2.0f - ubo.reso) / ubo.reso.y;
  const vec3 camera = vec3(0.0f, 0.0f, -1.0f);
  const vec3 rd = normalize(vec3(uv, 1.0f));

  float d = ray_march(camera, rd);

  if (d > 100.0) {
    frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    frag_color = vec4(1.0f, 1.0f, 1.0f, 0.0f);
  }
}
