#version 430

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

// This is a global that stores ALL the possible visible lights (1024 per tile)
/*
layout(std430, binding = 1) buffer VisibleLightIndicesBuffer {
	int data[];
} visibleLightIndicesBuffer;
*/

layout(std430, binding = 1) buffer VisibleLightIndicesBuffer{
	VisibleIndex data[];
} visibleLightIndicesBuffer;

// Uniforms
uniform sampler2D u_depthTexture; // this should be global right?
uniform mat4 view;
uniform mat4 projection;
uniform mat4 viewProjection;
uniform vec2 screenSize;

uniform int lightCount;


// Shared values between all the threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
// For the light indices, is it a shared local thing for now and then at the end we write to the others?

// Shared local storage for visible indices, will be written out to the buffer at the end
shared int visibleLightIndices[1024];

shared vec4 frustumPlanes[6];

#define BLOCK_SIZE 16
layout(local_size_x = BLOCK_SIZE, local_size_y = BLOCK_SIZE) in;
void main() {
	// Thread setup
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);
	ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
	ivec2 tileID = ivec2(gl_WorkGroupID.xy);
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
	int index = tileID.y * tileNumber.x + tileID.x; //is this the tileID or thread id?

	// initialize the shadered global values if we are the first thread
	if(gl_LocalInvocationIndex == 0) {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;
		//visibleLightIndicesBuffer.data[0].index = 0;
		//return;
		for (int i = 0; i < 1024; i++) {
			visibleLightIndices[i] = -1;
		}
		
	}

	// Sync threads
	barrier();

	float maxDepth, minDepth; // this should be in front of the barrier. Should it have some default value?
	// step 1 is to calculate the min and max depth of this tile
	vec2 text = vec2(location) / screenSize;
	float depth = texture(u_depthTexture, text).r; // TODO: Is this line right? What is text doing? What is R? (Could it just be 1 instaed of text? Kinda confused
	// Wait this seems all messed up
	uint depthInt = floatBitsToUint(depth);
	atomicMax(maxDepthInt, depthInt);
	atomicMin(minDepthInt, depthInt);

	// Sync threads
	barrier();

	// Step 2 is to then calculate the frustrum (only if we are the first thread)
	
	if(gl_LocalInvocationIndex == 0) {
		maxDepth = uintBitsToFloat(maxDepthInt);
		minDepth = uintBitsToFloat(minDepthInt);

		vec2 negativeStep = (2.0 * vec2(tileID)) / vec2(tileNumber);
		vec2 positiveStep = (2.0 * vec2(tileID + ivec2(1, 1))) / vec2(tileNumber);

		// Set up starting values for planes using steps and min and max z values
		frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth); // Near
		frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); // Far

		// first four
		for(uint i = 0; i < 4; i++) {
			//frustumPlanes[i] *= viewProjection;
			frustumPlanes[i] = frustumPlanes[i] * viewProjection;
			frustumPlanes[i] /= length(frustumPlanes[i].xyz);
		}

		// 5 and 6
		frustumPlanes[4] = frustumPlanes[4] * view;
		frustumPlanes[4] /= length(frustumPlanes[4].xyz);
		frustumPlanes[5] = frustumPlanes[5] * view;
		frustumPlanes[5] /= length(frustumPlanes[5].xyz);
	}

	// Sync threads
	barrier();

	// cull lights as step 3
	// Ok this part is fucked
	// Getting the wrong light index. If I have the right index it works.
	// So how do I split this so that the threads are parallel around the lights?
	uint threadCount = BLOCK_SIZE * BLOCK_SIZE;
	uint passCount = (lightCount + threadCount - 1) / threadCount;
	// Now switch to the threads processing lights
	for (uint i = 0; i < passCount; i++) {
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex; // TODO: Is light index even right?



		lightIndex = min(lightIndex, lightCount - 1);




		if (gl_LocalInvocationIndex != 0) {
		//	lightIndex = 0;
		//	continue;
		}

		// linear interpolation
		// OK what is this?
		// Why are we interpolating?
		// Interpolating to smooth between light positions, maybe have to return to reactivate this later if we want animated lights
		//vec4 position = mix(lightBuffer.data[i].previous, lightBuffer.data[i].current, alpha);
		vec4 position = lightBuffer.data[lightIndex].position;
		float radius = lightBuffer.data[lightIndex].radius;
		radius = 100.0;

		// Check for intersections with every dimension of the frustrum
		float distance = 0.0;
		for(uint j = 0; j < 6; j++) {
			distance = dot(position, frustumPlanes[j]) + radius;

			if(distance <= 0.0) {
				break; // If one fails, then there is no intersection
			}
		}

		// If greater than zero, then it is a visible light
		if(distance > 0.0) {
			// SO this increments it but returns the original so we know where WE are putting it, without telling the others
			uint offset;
			offset = atomicAdd(visibleLightCount, 1);
			visibleLightIndices[offset] = int(lightIndex);
			//visibleLightIndices[offset] = 0;


			//offset = index * 1024;
			//visibleLightIndicesBuffer.data[offset].index = 0;
			//return;
			
		}
	}

	// Sync threads
	barrier();

	// I'm intending that one thread in this group is doing this, but is that actually what this code means?
	if(gl_LocalInvocationIndex == 0) {
		// One of the threads should write all the visible light indices to the proper buffer
		uint offset = index * 1024;
		// TODO: I should be able to just copy this in one call, look at later
		for (uint i = 0; i < 1024; i++) {
			visibleLightIndicesBuffer.data[offset + i].index = visibleLightIndices[i];
			//visibleLightIndicesBuffer.data[offset + i].index = -1;
			//visibleLightIndicesBuffer.data[offset + i].index = -2;
		}

		if (visibleLightCount != 1024) {
			// Unless we have totally filled the entire array, mark the end with -1
			// That way the accum shader will know where to stop
			//visibleLightIndicesBuffer.data[offset + visibleLightCount].index = -1;
		}

		//visibleLightIndicesBuffer.data[offset].index = 0;
	}

	//TODO: FOr testing, remove
	//visibleLightIndicesBuffer.data[offset].index = -1;
	//visibleLightIndicesBuffer.data[0].index = 0;
	//visibleLightIndicesBuffer.data[0].index = 0;
}
