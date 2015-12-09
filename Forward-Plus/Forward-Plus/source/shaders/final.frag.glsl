#version 430

in VERTEX_OUT{
	vec3 fragmentPosition;
	//vec3 normal;
	vec2 textureCoordinates;
	//vec3 tangentLightPosition;
	
	mat3 TBN;
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


// TODO: Will this be right? DO some of them have multiple normal maps?
uniform sampler2D texture_normal1; // how to tell if this is active or not?


uniform int numberOfTilesX;


//uniform vec3 u_viewPosition;


out vec4 fragColor;


float attenuate(vec3 ldir, float radius) {


	float atten = dot(ldir, ldir) / (100 * radius);
	atten = 1.0 / (atten * 15.0 + 1.0);
	float cutoff = 0.5;
	atten = (atten - cutoff) / (1 - cutoff);
	//atten = (atten - 0.0625) * 1.066666;

	return clamp(atten, 0.0, 1.0);
}

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

	fragColor = vec4(clamp(color.rgb, 0.0, 1.0), color.a);
	//fragColor = color;
	//vec4 test = vec4(vec3(float(i) / 2.0), 1.0);
	//fragColor = test;

	//fragColor = base_diffuse;
}
