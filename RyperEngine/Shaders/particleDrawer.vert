#version 330

layout (location = 0) in vec3 wPos;
layout (location = 1) in vec3 wDir;

uniform mat4 PV; //M == Identity 

out vec3 color;

void main()
{
    gl_Position = PV * vec4(wPos, 1);
	color = wDir;
}