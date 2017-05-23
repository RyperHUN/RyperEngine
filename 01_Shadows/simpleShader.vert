#version 430

in vec3 vs_in_pos;
in vec3 vs_in_normal;
in vec2 vs_in_tex;

uniform mat4 MVP;
uniform mat4 M;

out vec3 wFragPos;
out vec3 frag_normal;
out vec2 frag_tex;

void main()
{
	gl_Position = MVP*vec4( vs_in_pos, 1 );

	wFragPos = (M * vec4(vs_in_pos, 1)).xyz;
	frag_normal = vs_in_normal; // TODO Model inverse
	frag_tex = vs_in_tex;
}