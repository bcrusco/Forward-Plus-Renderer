#version 430

in VERTEX_OUT {
  vec3 fragmentPosition;
  vec3 normal;
  vec2 textureCoordinates;
} fragment_in;

struct ListHead {
  uvec4 startAndCount;
};

struct ListNode {
  uvec4 lightIndexAndNext;
};

struct PointLight {
  vec4 color;
  vec3 position;
  float radius;
};

layout(std140, binding = 0) buffer HeadBuffer {
  ListHead data[];
} headBuffer;

layout(std140, binding = 1) buffer NodeBuffer {
  ListNode data[];
} nodeBuffer;

layout(std140, binding = 2) buffer LightBuffer {
  PointLight data[];
} lightBuffer;

uniform int tileToLightMap[][];

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform float alpha;
uniform int numberOfTilesX;
uniform vec3 u_viewPosition;

out vec4 fragColor;

float attenuate(vec3 lightDirection, float radius) {
  float attenuation = dot(lightDirection, lightDirection) / radius;
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - 0.0625) * 1.066666;

	return clamp(attenuation, 0.0, 1.0);
}

layout(early_fragment_tests) in;
void main() {

  ivec2 location = ivec2(gl_FragCoord.xy);
  ivec2 tileID = location / ivec2(16, 16);
  int index = tileID.y * numberOfTilesX + tileID.x;

  vec4 base_diffuse = texture(texture_diffuse1, fragment_in.textureCoordinates);
  vec4 base_specular = texture(texture_specular1, fragment_in.textureCoordinates);
  vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 normal = normalize(fragment_in.normal);
	vec3 viewDirection = normalize(u_viewPosition - fragment_in.fragmentPosition);

  uint nodeID = headBuffer.data[index].startAndCount.x;
	uint count = headBuffer.data[index].startAndCount.y;

  for(uint i = 0; i < count; i++) {
    uint lightID = nodeBuffer.data[nodeID].lightIndexAndNext.x;
    nodeID = nodeBuffer.data[nodeID].lightIndexAndNext.y;

    vec4 lightColor = lightBuffer.data[lightID].color;
    vec4 lightPosition = mix(lightBuffer.data[lightID].previous, lightBuffer.data[lightID].current, alpha);
    float radius = lightBuffer.data[lightID].velocityRadius.w;

    vec3 lightDirection = lightPosition.xyz - fragment_in.fragmentPosition.xyz;
    float attenuation = attenuate(lightDirection, radius);
    lightDirection = normalize(lightDirection);
    vec3 halfway = normalize(lightDirection + viewDirection);

    float diffuse = max(dot(lightDirection, normal), 0);
    float specular = pow(max(dot(halfway, normal), 0), 80.0);
    vec3 irradiance = lightColor.rgb * ((base_diffuse.rgb * diffuse) + (base_specular.rgb * vec3(specular))) * attenuation;
    color.rgb += irradiance;
  }

  fragColor = color;
}
