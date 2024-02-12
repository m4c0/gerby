#version 450

layout(push_constant) uniform upc {
  vec2 center;
  float scale;
  float aspect;
} pc;

layout(location = 0) in vec2 delta;
layout(location = 1) in vec2 a;
layout(location = 2) in vec2 b;
layout(location = 3) in float w;
layout(location = 4) in float diam;

layout(location = 0) out vec2 f_delta;

void main() {
  vec2 ab = normalize(b - a);
  float cth = ab.x;
  float sth = -ab.y;

  mat2 rot = mat2(
    cth, -sth,
    sth, cth
  );

  // TODO: check if this "stretches" in diagonals
  vec2 d = delta;
  d = rot * d;
  d *= 0.1f * diam;

  vec2 p = mix(a, b, w);
  p += d;
  p -= pc.center;
  p /= pc.scale;
  p.x /= pc.aspect;
  p.y *= -1;

  f_delta = delta;
  gl_Position = vec4(p, 0, 1);
}
