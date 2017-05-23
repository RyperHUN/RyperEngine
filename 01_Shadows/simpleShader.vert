#version 430

in vec3 vs_in_pos;

uniform mat4 MVP;
uniform mat4 M;

out vec3 wFragPos;

void main()
{
	gl_Position = MVP*vec4( vs_in_pos, 1 );

	wFragPos = (M * vec4(vs_in_pos, 1)).xyz;
}