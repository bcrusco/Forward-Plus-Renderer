#version 430

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

//http://www.dice.se/news/directx-11-rendering-battlefield-3/

#define BLOCK_SIZE 16
layout(local_size_x = BLOCK_SIZE, local_size_y = BLOCK_SIZE, local_size_z = 1) in;
void main() {
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);
	ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
	ivec2 tileID = ivec2(gl_WorkGroupID.xy);
	ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
	int index = tileID.y * tileNumber.x + tileID.x; //is this the tileID or thread id?

	// initialize the shadered global values if we are the first thread
	if (gl_LocalInvocationIndex == 0) {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visibleLightCount = 0;
	}

	// Sync threads
	barrier();

	float maxDepth, minDepth; // this should be in front of the barrier. Should it have some default value?
	// step 1 is to calculate the min and max depth of this tile
	vec2 text = vec2(location) / screenSize;
	float depth = texture(u_depthTexture, text).r;
	depth = (0.5 * projection[3][2]) / (depth + 0.5 * projection[2][2] - 0.5); // Linearize the depth value we brought in (fixes issue where camera position affected culling)

	// Convert the depth to an int so we can take the atomic min and max
	uint depthInt = floatBitsToUint(depth);
	atomicMin(minDepthInt, depthInt);
	atomicMax(maxDepthInt, depthInt);
	
	// Sync threads
	barrier();

	// Step 2 is to then calculate the frustrum (only if we are the first thread)

	if (gl_LocalInvocationIndex == 0) {
		
		minDepth = uintBitsToFloat(minDepthInt);
		maxDepth = uintBitsToFloat(maxDepthInt);

		/*
		vec2 negativeStep = (2.0 * vec2(tileID)) / vec2(tileNumber);
		vec2 positiveStep = (2.0 * vec2(tileID + ivec2(1, 1))) / vec2(tileNumber);

		// Increasing / decreaseing these changes how it looks in a favorable way
		// Set up starting values for planes using steps and min and max z values
		frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
		frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
		frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth); // Near
		frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); // Far

		// first four
		for (uint i = 0; i < 4; i++) {
			frustumPlanes[i] *= viewProjection;
			frustumPlanes[i] /= length(frustumPlanes[i].xyz);
		}

		// 5 and 6
		frustumPlanes[4] = frustumPlanes[4] * view;
		frustumPlanes[4] /= length(frustumPlanes[4].xyz);
		frustumPlanes[5] = frustumPlanes[5] * view;
		frustumPlanes[5] /= length(frustumPlanes[5].xyz);
		*/


		// new frustum imp
		vec2 tileScale = vec2(800, screenSize[1] * 2) * (1.0 / float(2 * BLOCK_SIZE)); // is this right value?
		vec2 tileBias = tileScale - vec2(gl_WorkGroupID.xy); // // might be right now?

		vec4 col1 = vec4(-projection[0][0] * tileScale.x, projection[0][1], tileBias.x, projection[0][3]);

		vec4 col2 = vec4(projection[1][0], -projection[1][1] * tileScale.y, tileBias.y, projection[1][3]);

		vec4 col4 = vec4(projection[3][0], projection[3][1], -1.0, projection[3][3]);

		//Left plane
		frustumPlanes[0] = col4 + col1;

		//right plane
		frustumPlanes[1] = col4 - col1;

		//top plane
		frustumPlanes[2] = col4 - col2;

		//bottom plane
		frustumPlanes[3] = col4 + col2;

		//near
		frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth);

		//far
		frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth);

		for (int i = 0; i < 4; i++) {
			frustumPlanes[i] *= 1.0 / length(frustumPlanes[i].xyz);
		}

		frustumPlanes[4] = frustumPlanes[4] * projection;
		frustumPlanes[4] /= length(frustumPlanes[4].xyz);
		frustumPlanes[5] = frustumPlanes[5] * projection;
		frustumPlanes[5] /= length(frustumPlanes[5].xyz);
		

	}



	// Sync threads
	barrier();

	

	// cull lights as step 3
	// Getting the wrong light index. If I have the right index it works.
	// So how do I split this so that the threads are parallel around the lights?
	uint threadCount = BLOCK_SIZE * BLOCK_SIZE;
	uint passCount = (lightCount + threadCount - 1) / threadCount;
	// Now switch to the threads processing lights
	for (uint i = 0; i < passCount; i++) {

		// Isn't the way this loop is set up have the effect of adding more than one of the same light?


		uint lightIndex = i * threadCount + gl_LocalInvocationIndex; // TODO: Is light index even right?
		if (lightIndex >= lightCount) {
			break;
		}

		// Interpolating to smooth between light positions, maybe have to return to reactivate this later if we want animated lights
		//vec4 position = mix(lightBuffer.data[i].previous, lightBuffer.data[i].current, alpha);
		vec4 position = lightBuffer.data[lightIndex].position;
		float radius = lightBuffer.data[lightIndex].paddingAndRadius.w;

		// Check for intersections with every dimension of the frustrum
		float distance = 0.0;
		for (uint j = 0; j < 6; j++) {
			distance = dot(position, frustumPlanes[j]) + radius;
			//distance = dot(position, frustumPlanes[j]);

			if (distance <= 0.0) {
			//if (-radius > distance) {
				break; // If one fails, then there is no intersection
			}
		}

		// For debugging. Basically disabled culling, ensures all lights pass and are added
		//distance = 1.0;

		// If greater than zero, then it is a visible light
		if (distance > 0.0) {
		//if (-radius <= distance) {
			// SO this increments it but returns the original so we know where WE are putting it, without telling the others

			uint offset = atomicAdd(visibleLightCount, 1);
			visibleLightIndices[offset] = int(lightIndex);
		}
	}

	// Sync threads
	barrier();

	// I'm intending that one thread in this group is doing this, but is that actually what this code means?
	if (gl_LocalInvocationIndex == 0) {
		// One of the threads should write all the visible light indices to the proper buffer
		uint offset = index * 1024;
		// TODO: I should be able to just copy this in one call, look at later
		for (uint i = 0; i < visibleLightCount; i++) {
			visibleLightIndicesBuffer.data[offset + i].index = visibleLightIndices[i];
		}

		if (visibleLightCount != 1024) {
			// Unless we have totally filled the entire array, mark the end with -1
			// That way the accum shader will know where to stop
			visibleLightIndicesBuffer.data[offset + visibleLightCount].index = -1;
		}
	}
}
