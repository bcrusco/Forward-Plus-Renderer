#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VERTEX_OUT {
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoordinates;
	//vec3 tangentLightPosition;
	/*
	mat3 TBN;
	vec3 tangentViewPosition;
	vec3 tangentFragmentPosition;
	*/
} vertex_out;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;


//uniform vec3 u_lightPosition;


//uniform vec3 u_viewPosition;

void main() {

	gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);
	// Why did I write these to be different?
	vertex_out.fragmentPosition = vec3(u_model * vec4(position, 1.0));

	vertex_out.normal = transpose(inverse(mat3(u_model))) * normal;

	// Reverse normals when we are inside the cube
	// TODO: I can remove this now I think
	/*
	if(u_reverseNormals) {
	vertex_out.normal = transpose(inverse(mat3(u_model))) * (-1.0 * normal);
	}
	else {
	vertex_out.normal = transpose(inverse(mat3(u_model))) * normal;
	}
	*/

	vertex_out.textureCoordinates = texCoords;

	/*
	gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
	vertex_out.fragmentPosition = vec3(u_model * vec4(position, 1.0)); // it should be right this way...
	vertex_out.textureCoordinates = texCoords;

	mat3 normalMatrix = transpose(inverse(mat3(u_model)));
	vec3 tan = normalize(normalMatrix * tangent);
	vec3 bitan = normalize(normalMatrix * bitangent);
	vec3 norm = normalize(normalMatrix * normal);

	mat3 TBN = transpose(mat3(tan, bitan, norm));


	//vertex_out.tangentLightPosition = TBN * u_lightPosition;


	
	vertex_out.tangentViewPosition = TBN * u_viewPosition;
	vertex_out.tangentFragmentPosition = TBN * vertex_out.fragmentPosition;
	vertex_out.TBN = TBN;
	*/
}
