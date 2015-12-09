#version 330 core

in vec3 position;
in vec3 color;
in vec3 radii;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 outColor;
out vec2 outTexCoords;

void main()
{
    //built-in things to pass down the pipeline
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0); //this can't be right?
    gl_PointSize = 7;
    outColor = vec3(1.0);
}
