#version 130

// VBO-b�l �rkez� v�ltoz�k
in vec3 vs_in_pos;

void main()
{
	gl_Position = vec4( vs_in_pos.xy,0.9999, 1 );
}