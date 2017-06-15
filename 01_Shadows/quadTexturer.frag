#version 430

in vec2 fragTex;

uniform sampler2D loadedTex;
uniform bool isInvertY;

out vec4 fs_out_col;

/////////////////////////////////////////
//////Post processing//////////////
vec4 InvertColor (vec2 tex)
{
	return vec4(vec3(1.0) - vec3(texture(loadedTex, tex)), 1.0);
}
//

void main()
{   
	vec2 tex = fragTex;
	if(isInvertY)
		tex = vec2(fragTex.x, 1.0 - fragTex.y);

	//fs_out_col = InvertColor (tex);
	fs_out_col = texture(loadedTex, tex);
}  