#version 330

out vec4 fs_out_col;
//TODO Coloring

in vec3 color;

///////////////////////////////////
////////--------UTILITY-FUNCTIONS

vec2 UvToNdc (vec2 uv)
{
	return vec2(uv.x * 2 - 1.0f, uv.y * -2.0f + 1.0f);
}

vec2 NdcToUV (vec2 ndc)
{
	return vec2((ndc.x + 1.0f)/2.0f,(ndc.y - 1.0f)/-2.0f);
}

vec2 HomogenToUV (vec4 hPos)
{
	vec2 ndc = hPos.xy / hPos.w;
	return NdcToUV (ndc);
}

void main()
{
	fs_out_col = vec4(abs(color),1);
}