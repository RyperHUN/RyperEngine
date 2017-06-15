#version 430

in vec2 fragTex;

uniform sampler2D loadedTex;
uniform bool isInvertY;

out vec4 fs_out_col;

/////////////////////////////////////////
//////Post processing//////////////
const float offset = 1.0 / 300.0;  
vec2 offsets[9] = vec2[]
(
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right    
);

float ShaerpenKernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
);
float BlurKernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);

float EdgeDetectKernel[9] = float[](
	1, 1, 1,
	1, -8, 1,
	1, 1, 1
);

vec4 KernelProcess(vec2 tex,float kernel[9])
{
	vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
        sampleTex[i] = vec3(texture(loadedTex, tex + offsets[i]));

	vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += sampleTex[i] * kernel[i];

	return vec4(color, 1);
}

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

	fs_out_col = KernelProcess(tex, EdgeDetectKernel);
	//fs_out_col = InvertColor (tex);
	//fs_out_col = Grayscale(tex);
	//fs_out_col = texture(loadedTex, tex);
}  