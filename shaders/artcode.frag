#version 450

layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;float ppi;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;float stroke;float rotate;int fill;int skew;} constant;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;

void main() {
  vec3 color = constant.color.rgb;
  // convert color to linear space using gamma correction 2.2
  color = pow(color, vec3(2.2));

  float alpha = constant.color.a;
  // uses loop-blinn for quadratic curves
  if (uv.x != 10.0f || uv.y != 10.0f) {
    // quadratic bezier
    float f = uv.x * uv.x - uv.y;
    float fw = fwidth(f);

    alpha = constant.color.a - smoothstep(-fw, fw, f);
  }

  // only renders the curve inside the triangle
  if(alpha < 0.001f) discard;

  frag_color = vec4(color, alpha);
}
