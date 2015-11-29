#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out VERTEX_OUT {
  vec3 fragmentPosition;
  vec3 normal;
  vec2 textureCoordinates;
} vertex_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main() {
  vertex_out.textureCoordinates = texCoords;
  gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
}
