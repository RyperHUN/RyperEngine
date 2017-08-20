#version 430

in VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	flat int instanceId;
	flat ivec4 texId;
	flat int cubeSide;
} FS;

uniform sampler2DArray tex1;
uniform sampler2D shadowMap;

out vec4 fs_out_col;

float ShadowCalcWithPcf(vec4 fragPosLightSpace)
{
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range for texture coord
    projCoords = projCoords * 0.5 + 0.5;
	//Percentage Closer Filtering
	float shadowMapSizePixel = 4096.0;
	float texelSize = 1.0 / shadowMapSizePixel;
	int pcfSize = 1;

	float bias = 0.005;
	int sumShadowPixels = (pcfSize * 2 + 1) * (pcfSize * 2 + 1);
	float notInShadowTexel = 0;
	float currentDepth = projCoords.z;
	for(int i = -pcfSize; i <= pcfSize; i++)
	{
		for(int j = -pcfSize; j <= pcfSize; j++)
		{
			vec2 offset = vec2(i, j) * texelSize;
			// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
			float shadowDepth = texture(shadowMap, projCoords.xy + offset).r; 
			if(currentDepth - bias < shadowDepth)
				notInShadowTexel+= 1;
		}
	}
	return float(notInShadowTexel) / float(sumShadowPixels);
}

void main()
{
	vec3 normal = normalize (FS.normal);
	vec3 color = mix(normal, vec3(FS.texCoord, 0), 0.5);

	fs_out_col = vec4(color, 1);
	
	int id = FS.texId[FS.cubeSide];
	vec4 default_color = texture(tex1, vec3(FS.texCoord, id));
	fs_out_col = default_color;

	//float ka = 0.1;
	//float lightValue = ShadowCalcWithPcf (FS.fragPosLightSpace4);
	//fs_out_col = ka * default_color + lightValue * default_color;

	//fs_out_col = vec4(FS.instanceId * 0.001, 0, 0, 1);
	//fs_out_col = vec4(FS.texId.x* 0.0001,0,0,1);
	//fs_out_col = vec4(0, FS.cubeSide * 0.3, 0, 1);
}

//texId
// Top texId.z
// ------
// |	|   Side: texId.y;
// |	|
// |	|
// ------
// bottom: texId.x
// texId.w = extra, for example block destruction