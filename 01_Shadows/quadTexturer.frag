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

vec4 Grayscale (vec2 tex)
{
	vec4 color = texture(loadedTex, tex);
	//float average = (color.r + color.g + color.b) / 3.0;
	//Weighted average gives better results
	float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b; 
    color = vec4(average, average, average, 1.0);
	return color;
}
//

void main()
{   
	vec2 tex = fragTex;
	if(isInvertY)
		tex = vec2(fragTex.x, 1.0 - fragTex.y);

	//fs_out_col = InvertColor (tex);
	fs_out_col = Grayscale(tex);
	//fs_out_col = texture(loadedTex, tex);
}  