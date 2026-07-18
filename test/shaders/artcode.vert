#version 450
layout(binding = 0) uniform ArtboardBuffer {mat4 proj;mat4 view;mat4 model;vec2 reso;vec2 viewport;} ubo;
layout(location = 0) in vec2 in_pos;
layout(location = 0) out vec2 art_pos;
void main() {
art_pos = in_pos;
art_pos.y = ubo.reso.y - art_pos.y;
gl_Position = ubo.proj * ubo.view * (ubo.model * vec4(art_pos, 0.0f, 1.0f));
}