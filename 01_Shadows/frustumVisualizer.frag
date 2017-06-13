#version 430

out vec4 fs_out_col;

uniform bool isSelected;

void main()
{	
		fs_out_col = vec4(1,1,0,0.2);
}