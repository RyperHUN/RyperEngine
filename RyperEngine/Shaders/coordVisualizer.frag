#version 430

in vec3 color;

out vec4 fs_out_col;

void main()
{   
	fs_out_col = vec4(color, 1);
}  