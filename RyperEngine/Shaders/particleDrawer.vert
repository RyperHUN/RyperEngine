#version 330

layout (location = 0) in vec3 wPos;

uniform mat4 PV; //M == Identity 

void main()
{
    gl_Position = PV * vec4(wPos, 1);
}