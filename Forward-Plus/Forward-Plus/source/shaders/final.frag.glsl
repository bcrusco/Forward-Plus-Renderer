#version 430

in VERTEX_OUT {
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoordinates;
} fragment_in;

struct PointLight {
	vec4 color;
	vec4 position;
	float radius;
};

struct VisibleIndex {
	int index;
};

layout(std430, binding = 0) buffer LightBuffer{
	PointLight data[];
} lightBuffer;

/*
// This is a global that stores ALL the possible visible lights (1024 per tile)
layout(std430, binding = 1) buffer VisibleLightIndicesBuffer{
	int data[];
} visibleLightIndicesBuffer;
*/

layout(std430, binding = 1) buffer VisibleLightIndicesBuffer{
	VisibleIndex data[];
} visibleLightIndicesBuffer;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform int numberOfTilesX;
uniform vec3 u_viewPosition;

out vec4 fragColor;

float attenuate(vec3 lightDirection, float radius) {
	float attenuation = dot(lightDirection, lightDirection) / radius;
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - 0.0625) * 1.066666;
	
	return clamp(attenuation, 0.0, 1.0);
}

// TODO: What is this one again? I don't think it is needed
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

	// Ok so in my version I won't know what the size of the data is (just the max)
	// So I have to assume that there's some default value that the array is set to?


	// So loop until we have covered the entire max lights or we found the end (because we found -1)
	// Hold up. I also need the offset cause this is the global buffer, not the shared per tile one

	uint offset = index * 1024;
	for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++) {
		uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;

		vec4 lightColor = lightBuffer.data[lightIndex].color;
		//vec4 lightPosition = mix(lightBuffer.data[lightID].previous, lightBuffer.data[lightID].current, alpha);
		vec4 lightPosition = lightBuffer.data[lightIndex].position;
		float lightRadius = lightBuffer.data[lightIndex].radius;

		vec3 lightDirection = lightPosition.xyz - fragment_in.fragmentPosition.xyz;
		//float attenuation = attenuate(lightDirection, lightRadius);
		float attenuation = 0.001;
		lightDirection = normalize(lightDirection);
		vec3 halfway = normalize(lightDirection + viewDirection);

		float diffuse = max(dot(lightDirection, normal), 0);
		float specular = pow(max(dot(halfway, normal), 0), 80.0);
		vec3 irradiance = lightColor.rgb * ((base_diffuse.rgb * diffuse) + (base_specular.rgb * vec3(specular))) * attenuation;
		color.rgb += irradiance;
		//color.rgb = irradiance;

		//color = vec4(1.0);
		//color = vec4(vec3(attenuation), 1.0);
	}
	// TODO: Gotta add some ambient component
	
	fragColor = color;
}
