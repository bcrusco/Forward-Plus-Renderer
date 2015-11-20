#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 u_shadowTransforms[6];

out vec4 fragPosition;

void main() {
  // Want to loop through the 6 faces we want to render
  for(int i = 0; i < 6; ++i) {
    gl_Layer = i;

    // Loop for each triangle's vertices
    for(int j = 0; j < 3; ++j) {
      fragPosition = gl_in[j].gl_Position;
      gl_Position = u_shadowTransforms[i] * fragPosition;
      EmitVertex(); // Emit the output variables to the current output primitive
    }

    EndPrimitive();
  }
}
