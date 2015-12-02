#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

//out vec2 o_textureCoordinates; // Shouldn't need this anymore?

out VERTEX_OUT {
  vec3 fragmentPosition;
  vec3 normal;
  vec2 textureCoordinates;
} vertex_out;

uniform bool u_reverseNormals;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main() {
  gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);
  // Why did I write these to be different?
  vertex_out.fragmentPosition = vec3(u_model * vec4(position, 1.0));

  // Reverse normals when we are inside the cube
  // TODO: I can remove this now I think
  if(u_reverseNormals) {
    vertex_out.normal = transpose(inverse(mat3(u_model))) * (-1.0 * normal);
  }
  else {
    vertex_out.normal = transpose(inverse(mat3(u_model))) * normal;
  }

  vertex_out.textureCoordinates = texCoords;
}
