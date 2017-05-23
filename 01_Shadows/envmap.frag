#version 130

// kimen� �rt�k - a fragment sz�ne
out vec4 fs_out_col;
in vec4 viewDir;

uniform samplerCube texCube;

void main()
{

	//fs_out_col = vec4(normalize(viewDir.xyz),1);
	fs_out_col = texture(texCube, normalize(viewDir.xyz));
}