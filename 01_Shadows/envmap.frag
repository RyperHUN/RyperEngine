#version 130

// kimen� �rt�k - a fragment sz�ne
out vec4 fs_out_col;
in vec4 viewDir;

uniform samplerCube skyBox;

void main()
{

	//fs_out_col = vec4(normalize(viewDir.xyz),1);
	fs_out_col = texture(skyBox, normalize(viewDir.xyz));
}