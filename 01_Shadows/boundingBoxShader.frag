#version 430

out vec4 fs_out_col;

uniform bool isSelected;

void main()
{
	
	if(isSelected)
		fs_out_col = vec4(0,0,1,0.3);
	else
		fs_out_col = vec4(1,0,0,0.3);
}