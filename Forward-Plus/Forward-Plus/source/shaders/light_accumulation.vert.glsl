#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VERTEX_OUT {
	vec3 fragmentPosition;
	vec2 textureCoordinates;
	mat3 TBN;
	vec3 tangentViewPosition;
	vec3 tangentFragmentPosition;
} vertex_out;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPosition;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0);
	vertex_out.fragmentPosition = vec3(model * vec4(position, 1.0));
	vertex_out.textureCoordinates = texCoords;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 tan = normalize(normalMatrix * tangent);
	vec3 bitan = normalize(normalMatrix * bitangent);
	vec3 norm = normalize(normalMatrix * normal);

	// For tangent space normal mapping
	mat3 TBN = transpose(mat3(tan, bitan, norm));
	vertex_out.tangentViewPosition = TBN * viewPosition;
	vertex_out.tangentFragmentPosition = TBN * vertex_out.fragmentPosition;
	vertex_out.TBN = TBN;
}
