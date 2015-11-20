#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 u_model;

void main() {
  gl_Position = u_model * vec4(position, 1.0);
}
