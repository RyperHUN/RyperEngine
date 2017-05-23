#version 130

in vec2 vs_in_pos;
in vec2 vs_in_tex;

out vec2 vs_out_tex;

uniform mat4 world;

void main()
{
	gl_Position = world * vec4( vs_in_pos,-1, 1 );
	vs_out_tex	= vs_in_tex;
}