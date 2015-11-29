#version 430

in VERTEX_OUT {
  vec3 fragmentPosition;
  vec3 normal;
  vec2 textureCoordinates;
} fragment_in;

out vec4 fragColor;

uniform sampler2D texture_diffuse1;
uniform vec4 u_ambient;

void main() {
  vec3 color = texture(texture_diffuse1, fragment_in.textureCoordinates).rgb;
  fragColor = vec4(color, 1.0) * u_ambient;
}
