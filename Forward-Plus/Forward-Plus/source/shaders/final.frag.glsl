#version 430

in VERTEX_OUT{
	vec3 fragmentPosition;
	//vec4 fragmentPositionLightSpace;
	vec2 textureCoordinates;
	mat3 TBN;
	//vec3 tangentLightPosition;
	vec3 tangentViewPosition;
	vec3 tangentFragmentPosition;
	
} fragment_in;

struct PointLight {
	vec4 color;
	vec4 position;
	vec4 paddingAndRadius;
};

struct VisibleIndex {
	int index;
};

layout(std430, binding = 0) buffer LightBuffer {
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) buffer VisibleLightIndicesBuffer {
	VisibleIndex data[];
} visibleLightIndicesBuffer;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D shadowMap;

uniform int numberOfTilesX;

//uniform vec3 u_lightPosition; // this is for the directional light
//uniform vec3 u_lightDir; //TODO: Double check this stuff

out vec4 fragColor;


float attenuate(vec3 ldir, float radius) {
	float atten = dot(ldir, ldir) / (100 * radius);
	atten = 1.0 / (atten * 15.0 + 1.0);
	float cutoff = 0.5;
	atten = (atten - cutoff) / (1 - cutoff);
	//atten = (atten - 0.0625) * 1.066666;

	return clamp(atten, 0.0, 1.0);
}

// PCF shadow calculation
/*
float shadowCalculation(vec4 fragmentPositionLightSpace, vec3 normal) {
	// Do perspective divide, then transform to 0 - 1 range
	vec3 projectionCoordinates = fragmentPositionLightSpace.xyz / fragmentPositionLightSpace.xyz;
	projectionCoordinates = projectionCoordinates * 0.5 + 0.5;

	// Then we get the closest depth value from the light's perspective
	float closestDepth = texture(shadowMap, projectionCoordinates.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = projectionCoordinates.z;

	// Calculate the bias based on map resolution and slope
	// TODO: This might be incorrect
	vec3 lightDirection = normalize(u_lightPosition - fragment_in.tangentFragmentPosition);
	float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

	// Perform PCP and check whether the current fragment position is in shadow
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			float pcfDepth = texture(shadowMap, projectionCoordinates.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	// Clamp the shadow at 0 when we are outside the far plane region of the frustum
	if (projectionCoordinates.z > 1.0) {
		shadow = 0.0;
	}

	return shadow;
}
*/


// some function to do directional light with shadow map
//vec3?4? return color
/*
vec3 directionalLightCalculation(vec3 normal, vec3 viewDirection, vec4 base_diffuse, vec4 base_specular) {
	//TODO: Double check this!
	//wait it can't be right. but it is for shadows?
	vec3 lightDirection = normalize(fragment_in.tangentLightPosition - fragment_in.tangentFragmentPosition);
	//vec3 lightDirection = normalize(-u_lightDir);
	vec3 halfway = normalize(lightDirection + viewDirection);

	float diff = max(dot(lightDirection, normal), 0.0);
	float spec = pow(max(dot(halfway, normal), 0.0), 80.0); //replace with shininess property later

	vec3 ambient = 0.1 * base_diffuse.rgb; //light color?
	vec3 diffuse = base_diffuse.rgb * diff;
	vec3 specular = base_specular.rgb * spec;

	vec3 lightColor = vec3(1.0); // temp

	float shadow = shadowCalculation(fragment_in.fragmentPositionLightSpace, normal);
	shadow = min(shadow, 1.0); // reduce strength to allow for some diffuse and spec light in shadowed region (configure later)
	vec3 lighting = lightColor * (ambient + (1.0 - shadow) * (diffuse + specular));
	return lighting;
	//return (ambient + diffuse + specular) * lightColor;
}
*/

void main() {
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	int index = tileID.y * numberOfTilesX + tileID.x;

	vec4 base_diffuse = texture(texture_diffuse1, fragment_in.textureCoordinates);
	vec4 base_specular = texture(texture_specular1, fragment_in.textureCoordinates);
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	


	//vec3 normal = normalize(fragment_in.normal);
	// get the normal from the normal map in the range 0 to 1 and convert to -1 to 1
	// in tangent space
	vec3 normal = texture(texture_normal1, fragment_in.textureCoordinates).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	//vec3 normal = normalize(fragment_in.normal);

	//vec3 viewDirection = normalize(u_viewPosition - fragment_in.fragmentPosition);
	vec3 viewDirection = normalize(fragment_in.tangentViewPosition - fragment_in.tangentFragmentPosition);

	//do the directional light and add it to the color?
	//color.rgb += directionalLightCalculation(normal, viewDirection, base_diffuse, base_specular);


	// Ok so in my version I won't know what the size of the data is (just the max)
	// So I have to assume that there's some default value that the array is set to?


	// So loop until we have covered the entire max lights or we found the end (because we found -1)
	// Hold up. I also need the offset cause this is the global buffer, not the shared per tile one
	//visibleLightIndicesBuffer.data[offset + i].index != -1
	uint offset = index * 1024;
	uint i;
	for (i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++) {
		uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;

		vec4 lightColor = lightBuffer.data[lightIndex].color;




		//vec4 lightPosition = lightBuffer.data[lightIndex].position;
		vec3 tangentLightPosition = fragment_in.TBN * lightBuffer.data[lightIndex].position.xyz;

		//vec3 lightDirection = lightPosition.xyz - fragment_in.fragmentPosition;
		vec3 lightDirection = tangentLightPosition - fragment_in.tangentFragmentPosition; //do I normalize here? no..



		float lightRadius = lightBuffer.data[lightIndex].paddingAndRadius.w;

		
		float lightDistance = length(lightDirection);

		float attenuation = attenuate(lightDirection, lightRadius);
		lightDirection = normalize(lightDirection);


		vec3 halfway = normalize(lightDirection + viewDirection);

		float diffuse = max(dot(lightDirection, normal), 0);

		// How do I change the material propery for the spec exponent? is it the alpha of hte spec texture?
		float specular = pow(max(dot(halfway, normal), 0), 80.0); // why is this the value? it was 80
		vec3 irradiance = lightColor.rgb * ((base_diffuse.rgb * diffuse) + (base_specular.rgb * vec3(specular))) * attenuation;

		//color.rgb = lightColor.rgb * base_diffuse.rgb * diffuse;


		color.rgb += irradiance;
		//color.rgb += clamp(irradiance.rgb, 0.0, 1.0);
	}

	// What is the correct way to add an ambient component?
	//Don't I need the light color?
	//color += base_diffuse * 0.1;

	/*
	if (i == 0) {
		color = base_diffuse * 0.3;
		fragColor = color;
		fragColor = vec4(0.0, 1.0, 0.0, 1.0);
		return;
	}
	*/

	// possibly remove ambient since it won't be able to handle normals?
	color += base_diffuse * 0.1;


	
	// the alpha of the diffuse represents the texture mask
	color.a = base_diffuse.a;

	if (base_diffuse.a < 0.1) {
		discard;
	}

	//fragColor = vec4(clamp(color.rgb, 0.0, 1.0), color.a);
	fragColor = color;
	//vec4 test = vec4(vec3(float(i) / 2.0), 1.0);
	//fragColor = test;

	//fragColor = base_diffuse;
}
