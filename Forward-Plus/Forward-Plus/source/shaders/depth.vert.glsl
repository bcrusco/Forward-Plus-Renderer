#version 430

layout (location = 0) in vec3 position;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main() {
	gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
}
