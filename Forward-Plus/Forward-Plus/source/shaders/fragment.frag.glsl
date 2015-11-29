#version 330 core

out vec4 fragColor;

in VERTEX_OUT {
  vec3 fragmentPosition;
  vec3 normal;
  vec2 textureCoordinates;
} fragment_in;

//uniform sampler2D u_diffuseTexture;
uniform sampler2D texture_diffuse1;

uniform samplerCube u_depthMap;
uniform vec3 u_lightPosition;
uniform vec3 u_viewPosition;
uniform float u_farPlane;
uniform bool u_shadows;

// Array of offset directions used for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float shadowCalculation(vec3 fragPosition) {
  vec3 fragmentToLight = fragPosition - u_lightPosition;
  float currentDepth = length(fragmentToLight);
  float shadow = 0.0;
  float bias = 0.15;
  int samples = 20;
  float viewDistance = length(u_viewPosition - fragPosition);
  float diskRadius = (1.0 + (viewDistance / u_farPlane)) / 25.0;

  for(int i = 0; i < samples; ++i) {
    float closestDepth = texture(u_depthMap, fragmentToLight + gridSamplingDisk[i] * diskRadius).r;
    closestDepth *= u_farPlane; // undo the zero to 1 mapping from before

    if(currentDepth - bias > closestDepth) {
      shadow += 1.0f;
    }
  }

  shadow /= float(samples);
  return shadow;
}

void main() {
  vec3 color = texture(texture_diffuse1, fragment_in.textureCoordinates).rgb;
  vec3 normal = normalize(fragment_in.normal);

  //TODO: all of this is going to be changed to a lighting model for forward+
  vec3 lightColor = vec3(0.3); // temp. will not be hard coded
  vec3 ambient = 0.3 * color; // temp. will be done in separate pass
  vec3 lightDirection = normalize(u_lightPosition - fragment_in.fragmentPosition);
  float diff = max(dot(lightDirection, normal), 0.0);
  vec3 diffuse = diff * lightColor;

  vec3 viewDirection = normalize(u_viewPosition - fragment_in.fragmentPosition);
  vec3 reflectDirection = reflect(-lightDirection, normal);
  float spec = 0.0;
  vec3 halfway = normalize(lightDirection + viewDirection);
  spec = pow(max(dot(normal, halfway), 0.0), 64.0);
  vec3 specular = spec * lightColor;
  float shadow = u_shadows ? shadowCalculation(fragment_in.fragmentPosition) : 0.0;
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

  fragColor = vec4(lighting, 1.0);
}
