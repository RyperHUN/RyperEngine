#version 430

layout (location = 0) in vec3 vs_in_pos;

uniform mat4 PVM;

//Interface Block
//out VS_OUT 
//{
//	out vec3 wFragPos;
//} VS;

void main()
{
	gl_Position = PVM * vec4(vs_in_pos, 1);
}