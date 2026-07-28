#version 450
layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;vec2 viewport;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;float stroke;float rotate;int fill;int skew;} constant;
layout(location = 0) out vec4 frag_color;
void main() {
vec3 color = constant.color.rgb;
color = pow(color, vec3(2.2));
frag_color = vec4(color, constant.color.a);
}
