#version 150

uniform vec4 u_color;

in vec3 world_normal;
in vec3 light_direction;
in vec3 view_direction;

out vec3 color;

void main()
{
  vec3 n = normalize(world_normal);
  vec3 l = normalize(light_direction);
  vec3 v = normalize(view_direction);
  vec3 h = normalize(v + l);

  float diffuse = clamp(dot(l, n), 0.0, 1.0);
  float specular = pow(clamp(dot(h, n), 0.0, 1.0), 80.0);

  color.rgb = u_color.rgb * diffuse + vec3(s);
  color.a = u_color.a;
}
