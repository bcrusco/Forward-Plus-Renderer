#version 430

struct PointLight {
	vec4 color;
	vec3 position;
	float radius;
};

layout(std140, binding = 0) buffer LightBuffer {
  PointLight data[];
} lightBuffer;

// This is a global that stores ALL the possible visible lights (1024 per tile)
layout(std140, binding = 1) buffer VisibleLightIndicesBuffer{
	uint data[];
} visibleLightIndicesBuffer;


// Uniforms
uniform sampler2D u_depthTexture; // this should be global right?

uniform int tileToLightMap[][];


uniform mat4 view;
uniform mat4 projection;
uniform mat4 viewProjection;
uniform vec2 screenSize;
uniform float alpha;

uniform int lightCount;


// Shared values between all the threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
// For the light indices, is it a shared local thing for now and then at the end we write to the others?

shared uint visibleLightIndices[1024];

shared uint tileLightStart;
shared uint tileLightCount;
shared vec4 frustumPlanes[6];

layout (local_size_x = 16, local_size_y = 16) in;
void main() {
  // Thread setup
  ivec2 location = ivec2(gl_GlobalInvocationID.xy);
  ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
  ivec2 tileID = ivec2(gl_WorkGroupID.xy);
  ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
  int index = tileID.y * tileNumber.x + tileID.x;

  // initialize the shadered global values if we are the first thread
  if(gl_LocalInvocationIndex == 0) {
	  minDepthInt = 0xFFFFFFFF;
	  maxDepthInt = 0;
	  visibleLightCount = 0;
  }

  // Sync threads
  barrier();


  // step 1 is to calculate the min and max depth of this tile
  vec2 text = vec2(location) / screenSize;
  float depth = texture(u_depthTexture, text).r; // Is this line right? What is text doing?
  atomicMax(tileMaxZ, floatBitsToUint(maxDepthInt));
  atomicMin(tileMinZ, floatBitsToUint(minDepthInt));


  // Sync threads
  barrier();

  // Step 2 is to then calculate the frustrum (only if we are the first thread)
  if(gl_LocalInvocationIndex == 0) {
    maxZ = uintBitsToFloat(maxDepthInt);
	minZ = uintBitsToFloat(minDepthInt);

    vec2 negativeStep = (2.0 * vec2(tileID)) / vec2(tileNumber);
    vec2 positiveStep = (2.0 * vec2(tileID + ivec2(1, 1))) / vec2(tileNumber);

    // Set up starting values for planes using steps and min and max z values
    frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
    frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
    frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
    frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
    frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minZ); // Near
    frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxZ); // Far

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
  vec4 position;
  float radius;

  uint previous;
  uint next;
  uint lightsPerTile = max(numberOfLights / 256, 1);
  uint remainder = numberOfLights % 256;
  uint lightStart;
  uint lightEnd;

  // set up light start and end
  if(gl_LocalInvocationIndex < remainder) {
    lightsPerTile++;
    lightStart = lightsPerTile * gl_LocalInvocationIndex;
  }
  else {
    lightStart = remainder * (lightsPerTile + 1) + (gl_LocalInvocationIndex - remainder) * lightsPerTile;
  }

  lightEnd = min(lightStart + lightsPerTile, numberOfLights);

  for(uint i = lightStart; i < lightEnd; i++) {
    // linear interpolation
	  // OK what is this?
	  // Why are we interpolating?
    position = mix(lightBuffer.data[i].previous, lightBuffer.data[i].current, alpha);
    radius = lightBuffer.data[i].velocityRadius.w;

	// Check for intersections with every dimension of the frustrum
    float distance = 0.0;
    for(uint j = 0; j < 6; j++) {
      distance = dot(position, frustumPlanes[j]) + radius;

      if(distance <= 0) {
        break; // If one fails, then there is no intersection
      }
    }

	// If greater than zero, then it is a visible light
    if(distance > 0) {
      next = atomicCounterIncrement(nextInsertionPoint);
      previous = atomicExchange(tileLightStart, next);

      nodeBuffer.data[next].lightIndexAndNext = uvec4(i, previous, 0.0, 0.0);
      



		// SO this increments it but returns the original so we know where WE are putting it, without telling the others
		uint offset;
		offset = atomicAdd(tileLightCount, 1);
		visibleLightIndices[offset] = lightIndex; //TODO: Where am I actualy storing this to?
    }
  }

  // Sync threads
  barrier();

	if(gl_LocalInvocationIndex == 0) {
		// One of the threads should write all the visible light indices to the proper buffer
		uint offset = index * 1024;
		// TODO: I should be able to just copy this in one call, look at later
		for (uint i = 0; i < 1024; i++) {
			visibleLightIndicies.data[offset + i] = visibleLightIndices[i];
		}
	}
}
