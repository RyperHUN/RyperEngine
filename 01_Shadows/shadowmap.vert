#version 130

in vec3 vs_in_pos;
out vec4 vs_out_pos;
uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4( vs_in_pos, 1 );
	vs_out_pos = gl_Position;
}