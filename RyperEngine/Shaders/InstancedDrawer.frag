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

///////////////////////////////////
////////--------STRUCTS-----------
///TODO Colors for lights
struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
	vec3 color;
}; 

struct DirLight {
    vec3 direction;

	vec3 color;
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

	vec3 color;
};  


uniform sampler2DArray tex1;
uniform sampler2D shadowMap;
uniform vec3 uwEye;

//TODO define only for max light, and upload used point light number
#define POINT_LIGHT_NUM 3

uniform SpotLight uSpotlight;
uniform DirLight uDirlight;
uniform PointLight uPointlights[POINT_LIGHT_NUM];

out vec4 fs_out_col;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoord)
{
	vec3 toLight = normalize(-light.direction);

	float diff = max(dot(normal, toLight), 0.0);

	vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 20);

	vec3 diffuse  = diff * light.color;

	return diffuse;
}

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

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range for texture coord
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float shadowDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Check whether current frag pos is in shadow
	float bias = 0.005;
    if(currentDepth - bias < shadowDepth)
		return 1.0;
	else
		return 0.0;
}  

void main()
{
	vec3 normal = normalize (FS.normal);
	vec3 color = mix(normal, vec3(FS.texCoord, 0), 0.5);
	vec3 viewDir = normalize (uwEye - FS.wFragPos);

	fs_out_col = vec4(color, 1);
	
	int id = FS.texId[FS.cubeSide];
	vec4 default_color = texture(tex1, vec3(FS.texCoord, id));

	float ka = 0.1;
	float lightValue = ShadowCalculation (FS.fragPosLightSpace4);
	vec3 kaColor = ka * default_color.xyz;
	vec3 kdColor = lightValue * calcDirLight (uDirlight,normal, viewDir, FS.texCoord) * default_color.xyz;
	fs_out_col = vec4(kaColor + kdColor, 1);

	//fs_out_col = vec4(FS.instanceId * 0.001, 0, 0, 1);
	//fs_out_col = vec4(FS.texId.x* 0.0001,0,0,1);
	//fs_out_col = vec4(0, FS.cubeSide * 0.3, 0, 1);
	//fs_out_col = vec4(lightValue , 0, 0,1);
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