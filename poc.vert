#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(location = 0) in vec2 delta;
layout(location = 1) in vec2 a;
layout(location = 2) in vec2 b;
layout(location = 3) in float w;

layout(location = 0) out vec2 f_delta;

void main() {
  vec2 ab = b - a;
  float cth = ab.x;
  float sth = -ab.y;

  mat2 rot = mat2(
    cth, -sth,
    sth, cth
  );

  vec2 d = delta;
  d = rot * d;
  d *= 0.1f;

  vec2 p = mix(a, b, w);
  p += d;
  p.x /= pc.aspect;

  f_delta = delta;
  gl_Position = vec4(p, 0, 1);
}
