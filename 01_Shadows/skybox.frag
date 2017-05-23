#version 430

in vec3 vs_out_tex;

out vec4 fs_out_col;

uniform samplerCube cubeTexture;

void main()
{
	fs_out_col = texture( cubeTexture, vs_out_tex );
}