#version 450

layout(location = 0) in vec2 f_delta;
layout(location = 1) in vec3 f_colour;

layout(location = 0) out vec4 frag_colour;

void main() {
  float d = length(f_delta);
  d = step(d, 1.0);
  
  frag_colour = vec4(f_colour, d);
}
