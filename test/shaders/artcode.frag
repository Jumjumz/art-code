#version 450
layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;vec2 viewport;} ubo;
layout(push_constant) uniform PushConstants {vec4 color;vec2 center;float stroke;float rotate;int fill;int skew;} constant;
layout(location = 0) out vec4 frag_color;
layout(location = 1) in vec3 data;
void main() {
vec3 color = constant.color.rgb;
color = pow(color, vec3(2.2));
if (constant.fill == 0) {frag_color = vec4(color, constant.color.a);}
else if (constant.fill == 1) {frag_color = vec4(color, constant.color.a);}
else {discard;}
}