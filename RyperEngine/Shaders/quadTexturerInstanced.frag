#version 430

in vec2 fragTex;
in float alpha;

uniform sampler2D loadedTex;
uniform bool isTexture;
uniform bool isAddedAlpha;
uniform vec4 uColor;

out vec4 fs_out_col;

void main()
{   
	if(!isTexture)
	{
		fs_out_col = uColor;
		return;
	}
	vec2 tex = fragTex;

	fs_out_col = texture(loadedTex, tex);
	if (isAddedAlpha)
		fs_out_col.w = fs_out_col.w * alpha;
}  