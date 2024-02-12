#version 450

layout(location = 0) in vec2 anchor;
layout(location = 1) in vec2 delta;

layout(location = 0) out vec2 f_delta;

void main() {
  f_delta = delta;
  gl_Position = vec4(anchor + delta * 0.1f, 0, 1);
}
