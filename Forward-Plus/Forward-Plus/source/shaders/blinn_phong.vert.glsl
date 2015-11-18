#version 150

in vec3 position;
in vec3 normal;

uniform mat4 u_world;
uniform mat4 u_view_projection;
uniform vec4 u_light_position;
uniform vec4 u_eye_position;

out vec3 world_normal;
out vec3 light_direction;
out vec3 view_direction;

void main()
{
  vec4 world_position = u_world * vec4(position, 1);

  light_direction = u_light_position.xyz - world_position.xyz;
  view_direction = u_eye_position.xyz - world_position.xyz;
  world_normal = (u_world * vec4(normal, 0)).xyz;

  gl_Position = u_view_projection * world_position;
}
