#version 130

// kimenõ érték - a fragment színe
out vec4 fs_out_col;
in vec4 viewDir;

uniform samplerCube texCube;

void main()
{

	//fs_out_col = vec4(normalize(viewDir.xyz),1);
	fs_out_col = texture(texCube, normalize(viewDir.xyz));
}