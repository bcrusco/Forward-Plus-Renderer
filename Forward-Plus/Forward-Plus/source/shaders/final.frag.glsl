#version 430

in VERTEX_OUT{
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoordinates;
} fragment_in;

struct PointLight {
	vec4 color;
	vec4 position;
	//float radius;
	vec4 paddingAndRadius;
};

struct VisibleIndex {
	int index;
};

layout(std430, binding = 0) buffer LightBuffer{
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) buffer VisibleLightIndicesBuffer{
	VisibleIndex data[];
} visibleLightIndicesBuffer;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform int numberOfTilesX;
uniform vec3 u_viewPosition;

out vec4 fragColor;

//float attenuate(vec3 lightDirection, float radius) {
float attenuate(float lightDistance, float radius) {
	float d = max(lightDistance - radius, 0);
	float denom = d / radius + 1;
	float attenuation = 1 / (denom * denom);
	float cutoff = 0.001;
	attenuation = (attenuation - cutoff) / (1 - cutoff);
	attenuation = max(attenuation, 0);

	return attenuation;
}

// Forces the early-z test to the fragment shader
//layout(early_fragment_tests) in;
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
	//visibleLightIndicesBuffer.data[offset + i].index != -1
	uint offset = index * 1024;
	for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++) {
		uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;

		vec4 lightColor = lightBuffer.data[lightIndex].color;
		vec4 lightPosition = lightBuffer.data[lightIndex].position;
		float lightRadius = lightBuffer.data[lightIndex].paddingAndRadius.w;

		vec3 lightDirection = lightPosition.xyz - fragment_in.fragmentPosition;
		float lightDistance = length(lightDirection);
		float attenuation = attenuate(lightDistance, lightRadius);
		//float attenuation = attenuate(lightDirection, lightRadius);
		//attenuation = 1.0;
		lightDirection = normalize(lightDirection);
		vec3 halfway = normalize(lightDirection + viewDirection);

		float diffuse = max(dot(lightDirection, normal), 0);
		float specular = pow(max(dot(halfway, normal), 0), 80.0);
		vec3 irradiance = lightColor.rgb * ((base_diffuse.rgb * diffuse) + (base_specular.rgb * vec3(specular))) * attenuation;


		color.rgb += irradiance;
		//color.rgb += clamp(irradiance.rgb, 0.0, 1.0);
	}

	// What is the correct way to add an ambient component?
	//Don't I need the light color?
	color += base_diffuse * 0.5;

	// the alpha of the diffuse represents the texture mask
	color.a = base_diffuse.a;

	if (base_diffuse.a < 0.1) {
		discard;
	}

	//fragColor = vec4(clamp(color.rgb, 0.0, 1.0), 1.0);
	fragColor = color;

	//fragColor = base_diffuse;
}
