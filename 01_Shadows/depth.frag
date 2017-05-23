#version 130

in vec2 vs_out_tex;

out vec4 fs_out_col;

uniform sampler2D texImage;

void main()
{
	fs_out_col = texture(texImage, vs_out_tex);
}