#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(location = 0) in vec2 anchor;
layout(location = 1) in vec2 delta;

layout(location = 0) out vec2 f_delta;

void main() {
  vec2 p = anchor + delta * 0.1f;
  p.x /= pc.aspect;

  f_delta = delta;
  gl_Position = vec4(p, 0, 1);
}
