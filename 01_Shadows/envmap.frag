#version 130

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

uniform samplerCube texCube;

void main()
{
	//vec3 refl_dir = reflect(-to_eye, normal);
	//vec3 refr_dir = refract(-to_eye, normal, 1.02);

	//vec4 refl_col = texture(texCube, refl_dir);
	//vec4 refr_col = texture(texCube, refr_dir);

	fs_out_col = vec4(0,1,0,1);
}