#version 330 core

in vec3 position;
in vec3 color;
in float radii;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform vec3 u_viewPosition;

out vec3 outColor;
out vec2 outTexCoords;

// Tweak these constants
const float minPointScale = 0.5;
const float maxPointScale = 0.7;
const float maxDistance = 300.0;


void main() {
	// Need to calcualte a scale for the point based on distance from view
	float cameraDistance = distance(position, u_viewPosition);
	float pointScale = 1.0 - (cameraDistance / maxDistance);
	pointScale = max(pointScale, minPointScale);
	pointScale = min(pointScale, maxPointScale);

    gl_Position = u_projection * u_view * vec4(position, 1.0);

	gl_PointSize = radii * pointScale;

    outColor = color;
}
