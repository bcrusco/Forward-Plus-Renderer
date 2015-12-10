#version 430

layout (location = 0) in vec3 position;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0);
}
