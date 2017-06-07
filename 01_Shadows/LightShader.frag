#version 430

///TODo
//uniform vec3 color;

in  vec2 texCoord;
in vec3 normal;

out vec4 fs_out_col;

void main()
{   
	//fs_out_col = vec4(1.0);
	fs_out_col = vec4(abs(normal), 1.0);
}  