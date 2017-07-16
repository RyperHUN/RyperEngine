#version 430
layout (location = 0) in vec3 position;

uniform mat4 M;
uniform mat4 LightSpaceMtx;

void main()
{
    gl_Position = LightSpaceMtx * M * vec4(position, 1.0f);
}  