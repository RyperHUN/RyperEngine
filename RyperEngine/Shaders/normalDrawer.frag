#version 430

in VS_OUT 
{
	in vec3 wFragPos;
	in vec3 normal;
	in vec2 texCoord;
	in vec4 fragPosLightSpace4;
	in vec4 testColor;
} FS;

uniform vec3 color;

out vec4 fs_out_col;

void main()
{
	fs_out_col = vec4(0,1,1,1);
}
