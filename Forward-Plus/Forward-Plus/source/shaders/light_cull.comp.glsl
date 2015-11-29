#version 430

struct ListHead {
  uvec4 startAndCount;
};

struct ListNode {
  uvec4 lightIndexAndNext;
};

struct PointLight {
  vec4 color;
  vec4 previous;
  vec4 current;
  vec4 velocityRadius;
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

// Uniforms
layout(binding = 0) uniform sampler2D depthSampler;
layout(binding = 0) uniform atomic_uint nextInsertionPoint;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 viewProjection;
uniform vec2 clipPlanes;
uniform vec2 screenSize;
uniform float alpha;
uniform int numberOfLights;

// Shared values between all the threads
shared uint tileMinZ;
shared uint tileMaxZ;
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
    tileMinZ = 0x7F7FFFFF;
    tileMaxZ = 0;
    tileLightCount = 0;
    tileLightStart = 0xFFFFFFFF;
  }

  // Sync threads
  barrier();


  // step 1 is to calculate the min and max depth of this tile
  vec2 text = vec2(location) / screenSize;
  float depth = texture(depthSampler, text).r;
  float z = (0.5 * projection[3][2]) / (depth + 0.5 * projection[2][2] - 0.5);
  float maxZ = max(clipPlanes.x, z);
  float minZ = min(clipPlanes.y, z);

  if(minZ <= maxZ) {
    // Update tile min and maxes
    atomicMax(tileMaxZ, floatBitsToUint(maxZ));
    atomicMin(tileMinZ, floatBitsToUint(minZ));
  }

  // Sync threads
  barrier();

  // Step 2 is to then calculate the frustrum (only if we are the first thread)
  if(gl_LocalInvocationIndex == 0) {
    maxZ = uintBitsToFloat(tileMaxZ);
    minZ = uintBitsToFloat(tileMinZ);

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
    position = mix(lightBuffer.data[i].previous, lightBuffer.data[i].current, alpha);
    radius = lightBuffer.data[i].velocityRadius.w;

    float distance = 0.0;
    for(uint j = 0; j < 6; j++) {
      distance = dot(position, frustumPlanes[j]) + radius;

      if(distance <= 0) {
        break;
      }
    }

    if(distance > 0) {
      next = atomicCounterIncrement(nextInsertionPoint);
      previous = atomicExchange(tileLightStart, next);

      nodeBuffer.data[next].lightIndexAndNext = uvec4(i, previous, 0.0, 0.0);
      atomicAdd(tileLightCount, 1);
    }
  }

  // Sync threads
  barrier();

  if(gl_LocalInvocationIndex == 0) {
    headBuffer.data[index].startAndCount = uvec4(tileLightStart, tileLightCount, 0.0, 0.0);
  }
}
