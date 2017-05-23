#version 430

in vec3 vs_in_pos;

out vec3 vs_out_tex;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP*vec4( vs_in_pos, 1 );
	vs_out_tex = vs_in_pos;
}