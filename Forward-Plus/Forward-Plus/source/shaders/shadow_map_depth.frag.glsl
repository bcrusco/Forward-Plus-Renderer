#version 330 core

in vec4 fragPosition;

uniform vec3 u_lightPosition;
uniform float u_farPlane;

void main() {
  // Map the light's distance to a range between 0 and 1
  float lightDistance = length(fragPosition.xyz - u_lightPosition) / u_farPlane;

  gl_FragDepth = lightDistance;
}
