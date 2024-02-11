#version 450

layout(location = 0) in vec2 anchor;
layout(location = 1) in vec2 delta;

void main() {
  gl_Position = vec4(anchor + delta * 0.1f, 0, 1);
}
