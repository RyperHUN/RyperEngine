#version 430

in vec2 fragTex;
in float alpha;
in float texId;

uniform sampler2D loadedTex;
uniform sampler2DArray texArray;
uniform bool isTextureArray;
uniform bool isTexture;
uniform bool isAddedAlpha;
uniform vec4 uColor;

out vec4 fs_out_col;

void main()
{   
	if(isTextureArray)
	{
		fs_out_col = texture(texArray, vec3(fragTex,floor(texId)));
		fs_out_col.w = fs_out_col.w * alpha;
		return;
	}
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