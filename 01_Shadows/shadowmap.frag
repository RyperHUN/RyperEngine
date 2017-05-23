#version 130

in vec4 vs_out_pos;
out float fs_out_depth;

void main()
{
	fs_out_depth = vs_out_pos.z/vs_out_pos.w;
}